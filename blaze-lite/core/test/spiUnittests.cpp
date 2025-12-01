/*
    This file contains the unit tests for the spiFlash class. 
*/
#include "spiFlash.h"

using std::cout, std::endl, std::cin;
/*
tests:
1. write
2. kwrite

3. buffer - 1 small msg
4. flush
5. kLog - 1 small msg
6. kflush

7. buffer - 2 small msgs
8. kLog - 2 small msgs

9. buffer - 1 large msg
10. kLog - 1 large msg

11. buffer - 1 xl msg
12. kLog - 1 xl msg

13. buffer - 2 large msgs
14. kLog - 2 large msgs

15. buffer - 1 xl msg -> 1 l msg
16. kLog - 1 xl msg -> 1 l msg

17. queue
18. tick 
*/

#define GREETING "Hello World!\n"
#define MSG1 "This is the first msg.\n"
#define MSG2 "This is the second msg.\n"
#define MSGL msgl
#define MSGL2 msgl2
#define MSGXL msgxl

#define BUFF_SIZE 256

int main () {
    char wait;
    char msgl[BUFF_SIZE];
    char msgl2[BUFF_SIZE];
    char msgxl[BUFF_SIZE + 8];
    
    memset(msgl, '1', sizeof(msgl));
    memset(msgl2, '2', sizeof(msgl2));
    memset(msgxl, 'X', sizeof(msgxl));
    
    spiFlash f(0, BUFF_SIZE, BUFF_SIZE);
    

    f.write(sizeof(GREETING), GREETING);
    cout << "test 1 done" << endl;
    cin >> wait;

    f.kwrite(sizeof(GREETING), GREETING);
    cout << "test 2 done" << endl;
    cin >> wait;

    f.buffer(sizeof(MSG1), MSG1);
    cout << "test 3 done" << endl;
    cin >> wait;

    f.flush();
    cout << "test 4 done" << endl;
    cin >> wait;

    f.kLog(sizeof(MSG1), MSG1);
    cout << "test 5 done" << endl;
    cin >> wait;

    f.kflush();
    cout << "test 6 done" << endl;
    cin >> wait;

    f.buffer(sizeof(MSG1), MSG1);
    f.buffer(sizeof(MSG2), MSG2);
    f.flush();
    cout << "test 7 done" << endl;
    cin >> wait;

    f.kLog(sizeof(MSG1), MSG1);
    f.kLog(sizeof(MSG2), MSG2);
    f.kflush();
    cout << "test 8 done" << endl;
    cin >> wait;

    f.buffer(sizeof(MSGL), MSGL);
    f.flush();
    cout << "test 9 done" << endl;
    cin >> wait;

    f.kLog(sizeof(MSGL), MSGL);
    f.kflush();
    cout << "test 10 done" << endl;
    cin >> wait;

    f.buffer(sizeof(MSGXL), MSGXL);
    f.flush();
    cout << "test 11 done" << endl;
    cin >> wait;

    f.kLog(sizeof(MSGXL), MSGXL);
    f.kflush();
    cout << "test 12 done" << endl;
    cin >> wait;

    f.buffer(sizeof(MSGL), MSGL);
    f.buffer(sizeof(MSGL2), MSGL2);
    f.flush();
    cout << "test 13 done" << endl;
    cin >> wait;

    f.kLog(sizeof(MSGL), MSGL);
    f.kLog(sizeof(MSGL2), MSGL2);
    f.kflush();
    cout << "test 14 done" << endl;
    cin >> wait;

    f.buffer(sizeof(MSGXL), MSGXL);
    f.buffer(sizeof(MSGL2), MSGL2);
    f.flush();
    cout << "test 15 done" << endl;
    cin >> wait;

    f.kLog(sizeof(MSGXL), MSGXL);
    f.kLog(sizeof(MSGL2), MSGL2);
    f.kflush();
    cout << "test 16 done" << endl;
    cin >> wait;
}