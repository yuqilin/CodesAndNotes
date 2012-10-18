
#include <iostream>
using namespace std;

template <typename T>
class CPoint {
public:
	T m_x;
	T m_y;
};

int main() {
	CPoint<int> objPoint;
	cout << "Size of object is = " << sizeof(objPoint) << endl;
	cout << "Address of object is = " << &objPoint << endl;
	return 0;
}