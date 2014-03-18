#include <android/bitmap.h>
#include <stdlib.h>
#include "com_myandroid_paintboard_ImageProc.h"
#include "jni_log.h"

typedef struct CvFFillSegment {
	uint16_t y;
	uint16_t l;
	uint16_t r;
	uint16_t prevl;
	uint16_t prevr;
	short dir;
} CvFFillSegment;

#define DIR_UP				1		// direction up
#define DIR_DOWN			-1		// direction down
#define BOUNDARY_COLOR		0xff000000		// black color
#define MAX(a, b)			((a) > (b) ? (a) : (b))

#define LOBYTE(w)           ((uint8_t)(((uint32_t)(w)) & 0xff))
#define HIBYTE(w)           ((uint8_t)((((uint32_t)(w)) >> 8) & 0xff))
#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((uint16_t)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))
#define ABS(a)				( (a) > 0 ? (a) : -(a))

#define ICV_PUSH( Y, L, R, PREV_L, PREV_R, DIR )\
{                                               \
    tail->y = (uint16_t)(Y);                    \
    tail->l = (uint16_t)(L);                    \
    tail->r = (uint16_t)(R);                    \
    tail->prevl = (uint16_t)(PREV_L);           \
    tail->prevr = (uint16_t)(PREV_R);           \
    tail->dir = (short)(DIR);                   \
    if( ++tail >= buffer_end )                  \
        tail = buffer;                          \
}

#define ICV_POP( Y, L, R, PREV_L, PREV_R, DIR ) \
{                                               \
    Y = head->y;                                \
    L = head->l;                                \
    R = head->r;                                \
    PREV_L = head->prevl;                       \
    PREV_R = head->prevr;                       \
    DIR = head->dir;                            \
    if( ++head >= buffer_end )                  \
        head = buffer;                          \
}

#define ICV_SET_C4( p, q ) \
		((p)[0] = (q)[0], (p)[1] = (q)[1], (p)[2] = (q)[2], (p)[3] = (q)[3])

//#define RGBA2ARGB( rgba ) \
//	( ((((uint32_t)(rgba)) & 0xff) << 24) | (((uint32_t)(rgba)) >> 8) )

#define TANS_ARGB_RGBA( pixel ) \
	{uint8_t temp = (pixel)[0]; (pixel)[0] = (pixel)[2]; (pixel)[2] = temp; }

#define ARGB_SETBY_RGBA( argb, rgba ) \
		((argb)[0] = (rgba)[2], (argb)[1] = (rgba)[1], (argb)[2] = (rgba)[0], (argb)[3] = (rgba)[3])

#define RGBA_SETBY_ARGB( rgba, argb ) \
		((rgba)[0] = (argb)[2], (rgba)[1] = (argb)[1], (rgba)[2] = (argb)[0], (rgba)[3] = (argb)[3])

static void ScanLineSeedFill(uint8_t* pImage, int w, int h, int x, int y,
		int step, int new_color, int boundary_color);

static void ThrowException(JNIEnv* env, const char* className,
		const char* message);

static jboolean isPixelValid(int w, int h, int x, int y, int old_color,
		int cur_color, int new_color, int boundary_color);

static jboolean ColorComp(int a, int b);

JNIEXPORT jboolean JNICALL Java_com_myandroid_paintboard_ImageProc_imgproc(
		JNIEnv * env, jclass class, jobject src_bitmap, jint x, jint y,
		jint new_color) {

	jboolean isProcessed = JNI_FALSE;

	uint8_t* pixelsBuffer = 0;
	AndroidBitmapInfo srcInfo;

	LOGI("enter imgproc: x=%d, y=%d, new_color=%08x", x, y, new_color);

	if (0 > AndroidBitmap_getInfo(env, src_bitmap, &srcInfo)) {
		ThrowException(env, "java/io/IOException",
				"Unable to get bitmap info.");
		LOGE("Unable get bitmap info.");
		goto exit;
	}

	LOGI(
			"bitmap info width=%d, height=%d, format=%d, stride=%d, flags=%d", srcInfo.width, srcInfo.height, srcInfo.format, srcInfo.stride, srcInfo.flags);

	// Lock bitmap and get the raw bytes
	if (0 > AndroidBitmap_lockPixels(env, src_bitmap, (void**) &pixelsBuffer)) {
		ThrowException(env, "java/io/IOException", "Unable to lock pixels.");
		LOGE("Unable to lock pixels.");
		goto exit;
	}

	ScanLineSeedFill(pixelsBuffer, srcInfo.width, srcInfo.height, x, y,
			srcInfo.stride, new_color, BOUNDARY_COLOR);

	// Unlock bitmap
	if (0 > AndroidBitmap_unlockPixels(env, src_bitmap)) {
		ThrowException(env, "java/io/IOException", "Unable to unlock pixels.");
		LOGE("Unable to unlock pixels.");
		goto exit;
	}

	exit: return isProcessed;
}

static void ScanLineSeedFill(uint8_t* pImage, int w, int h, int x, int y,
		int step, int new_color, int boundary_color) {

	int buffer_size = MAX(w, h) * 2;
	CvFFillSegment* buffer = (CvFFillSegment*) malloc(
			sizeof(CvFFillSegment) * buffer_size);
	CvFFillSegment* buffer_end = buffer + buffer_size, *head = buffer, *tail =
			buffer;

	uint8_t* img = pImage + step * y;

	int i, j, k, L, R;
	int old_color = 0xff000000;
	int pixel;

	L = R = x;
	//old_color = img[L];
	//ICV_SET_C4( (uint8_t*)&old_color, img + L*4 );
	//old_color = RGBA2ARGB(*((uint32_t*)img + L));
	ARGB_SETBY_RGBA((uint8_t*)&old_color, img + L*4);

	uint8_t* byte = img + L * 4;
	LOGI(
			"img=%02x%02x%02x%02x, old_color = %08x, new_color=%08x", byte[0], byte[1], byte[2], byte[3], old_color, new_color);

	RGBA_SETBY_ARGB(img + L*4, (uint8_t*)&new_color);

	while (--L > 0) {
		//pixel = RGBA2ARGB(*((uint32_t*)img + L));
		ARGB_SETBY_RGBA((uint8_t*)&pixel, img + L*4);
		if (isPixelValid(w, h, L, y, old_color, pixel, new_color,
				boundary_color)) {
			//img[L] = new_color;
			//LOGD("[%d, %d]=%08x", L, y, pixel);
			RGBA_SETBY_ARGB(img + L*4, (uint8_t*)&new_color);
		} else {
			break;
		}
	}

	while (++R < w) {
		//ICV_SET_C4( (uint8_t*)&pixel, img + R*4 );
		//pixel = RGBA2ARGB(*((uint32_t*)img + R));
		ARGB_SETBY_RGBA((uint8_t*)&pixel, img + R*4);

		if (isPixelValid(w, h, R, y, old_color, pixel, new_color,
				boundary_color)) {
			//img[R] = new_color;
			//LOGD("[%d, %d]=%08x", R, y, pixel);
			RGBA_SETBY_ARGB(img + R*4, (uint8_t*)&new_color);
		} else {
			break;
		}
	}

	--R;
	++L;

	//LOGD("L=%d, R=%d", L, R);

	ICV_PUSH( y, L, R, R+1, R, DIR_UP);

	while (head != tail) {
		int k, YC, PL, PR, dir;
		ICV_POP( YC, L, R, PL, PR, dir);

		int _8_connectivity = 1;
		int data[][3] = { { -dir, L - _8_connectivity, R + _8_connectivity }, {
				dir, L - _8_connectivity, PL - 1 }, { dir, PR + 1, R
				+ _8_connectivity } };

		for (k = 0; k < 3; k++) {
			dir = data[k][0];
			img = pImage + (YC + dir) * step;

			int left = data[k][1];
			int right = data[k][2];

			if (YC + dir >= h)
				continue;

			for (i = left; i <= right; i++) {
				//pixel = RGBA2ARGB(*((uint32_t*)img + i));
				ARGB_SETBY_RGBA((uint8_t*)&pixel, img + i*4);
				if (i < w
						&& isPixelValid(w, h, i, YC + dir, old_color, pixel,
								new_color, boundary_color)) {
					j = i;
					//setPixelColor(i, YC + dir, new_color);
					//img[i] = new_color;
					RGBA_SETBY_ARGB(img + i*4, (uint8_t*)&new_color);

					//pixel = RGBA2ARGB(*((uint32_t*)img + j));
					ARGB_SETBY_RGBA((uint8_t*)&pixel, img + j*4);
					while (--j >= 0
							&& isPixelValid(w, h, j, YC + dir, old_color, pixel,
									new_color, boundary_color)) {
						//setPixelColor(j, YC + dir, new_color);
						//img[j] = new_color;
						RGBA_SETBY_ARGB(img + j*4, (uint8_t*)&new_color);
					}

					//pixel = RGBA2ARGB(*((uint32_t*)img + i));
					ARGB_SETBY_RGBA((uint8_t*)&pixel, img + i*4);
					while (++i < w
							&& isPixelValid(w, h, i, YC + dir, old_color, pixel,
									new_color, boundary_color)) {
						//setPixelColor(i, YC + dir, new_color);
						//img[i] = new_color;
						RGBA_SETBY_ARGB(img + i*4, (uint8_t*)&new_color);
					}
					ICV_PUSH( YC+dir, j+1, i-1, L, R, -dir);
				}
			}
		}
	}

	if (buffer)
		free(buffer);
}

/**
 * Throws a new exception using the given exception class
 * and exception message.
 *
 * @param env JNIEnv interface.
 * @param className class name.
 * @param message exception message.
 */
static void ThrowException(JNIEnv* env, const char* className,
		const char* message) {
	// Get the exception class
	jclass clazz = (*env)->FindClass(env, className);
	// If exception class is found
	if (0 != clazz) {
		// Throw exception
		(*env)->ThrowNew(env, clazz, message);
		// Release local class reference
		(*env)->DeleteLocalRef(env, clazz);
	}
}

static jboolean isPixelValid(int w, int h, int x, int y, int old_color,
		int cur_color, int new_color, int boundary_color) {
	jboolean bValid = JNI_FALSE;

	if (x < 0 || y < 0 || x >= w || y >= h || cur_color == new_color)
		return JNI_FALSE;

	jboolean bSimilarTarget = ColorComp(cur_color, old_color);
	jboolean bSimilarBoundary = ColorComp(cur_color, boundary_color);

	/*
	if (bSimilarBoundary) {
		LOGD("boundary true, cur_color=%08x, boundary_color=%08x", cur_color, boundary_color);
	} else {
		LOGD("boundary false, cur_color=%08x, boundary_color=%08x", cur_color, boundary_color);
	}
	//*/

	if (bSimilarTarget && !bSimilarBoundary) {
		bValid = JNI_TRUE;
	} else {
		bValid = JNI_FALSE;
	}
	return bValid;
}

static jboolean ColorComp(int a, int b) {
	uint8_t* rgbA = (uint8_t*) &a;
	uint8_t* rgbB = (uint8_t*) &b;
	int absR = ABS(rgbA[0] - rgbB[0]);
	int absG = ABS(rgbA[1] - rgbB[1]);
	int absB = ABS(rgbA[2] - rgbB[2]);
	//float distance = ((float)(absR * absR + absG * absG + absB * absB));
	int distance = absR + absG + absB;
	//if (distance - 10000.0f < 0.01)
	if (distance < 50)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}
