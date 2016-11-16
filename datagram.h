#ifndef DATAGRAM
#define DATAGRAM

#include <iostream>
#include <string>

#include "definitions.h"

using namespace std;

class IPAddress {
	private:
		unsigned int a[4];
	
	public:
        IPAddress();    
		int parse(string s);
		void display();
		int	sameAddress(IPAddress x);
        /******************new*********************/
        int	isNULL();
        /******************new*********************/
};


class datagram {
	private:
		IPAddress src, dest;
		int length;
		string msg;
	
	public:
		void makeDatagram(IPAddress s, IPAddress d, string m);
		void display();
};


#endif