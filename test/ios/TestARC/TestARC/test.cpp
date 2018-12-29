//
//  TestC.cpp
//  TestARC
//
//  Created by 孙磊 on 2018/2/3.
//  Copyright © 2018年 sohu. All rights reserved.
//

#include "test.h"

void *Test::p_ = NULL;
void Test::c_set(void *p) {
    p_ = p;
}

void *Test::c_get() {
    return p_;
}

void Test::clear() {
    p_ = nullptr;
}

void *Test::alloc() {
    return (void*)new A;
}
