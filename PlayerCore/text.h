
#ifndef _TEXT_H_
#define _TEXT_H_

template<class T, typename SEP>
T Explode(const T& str, CAtlList<T>& sl, SEP sep, size_t limit = 0)
{
	sl.RemoveAll();

	for (int i = 0, j = 0; ; i = j + 1) {
		j = str.Find(sep, i);

		if (j < 0 || sl.GetCount() == limit - 1) {
			sl.AddTail(str.Mid(i).Trim());
			break;
		} else {
			sl.AddTail(str.Mid(i, j - i).Trim());
		}
	}

	return sl.GetHead();
}


#endif