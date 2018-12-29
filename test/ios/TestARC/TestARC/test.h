//
//  TestC.hpp
//  TestARC
//
//  Created by 孙磊 on 2018/2/3.
//  Copyright © 2018年 sohu. All rights reserved.
//

#ifndef TestC_hpp
#define TestC_hpp

#include <stdio.h>

class A{
public:
    A() {
        printf("A created\n");
    }
    
    ~A() {
        printf("A deleted\n");
    }
};

class Test {
public:
    static void c_set(void *p);
    static void *c_get();
    static void clear();
    static void *alloc();
private:
    static void *p_;
};

#endif /* TestC_hpp */
