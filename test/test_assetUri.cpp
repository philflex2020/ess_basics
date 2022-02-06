/******************************************************
 *              
 *                 test_assetUri.h
 *  p.wilshire
 *   rework 02/05/2022
 *   test with test_assetUri
 *
 *  added setParam("foo[1]", true);
 *      sets a bit in a param
 * 
 *    
 ******************************************************/



#include "assetUri.h"


 int main(int argc, char*argv[])
 {
    std::cout << " hello" << std::endl;
    assetUri my("/foo:one:two@p");
    my.show();


    return 0;
 }
 