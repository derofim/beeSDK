//
//  ViewController.m
//  TestARC
//
//  Created by 孙磊 on 2018/2/3.
//  Copyright © 2018年 sohu. All rights reserved.
//

#import "ViewController.h"
#include "test.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>

@class MyObj;

@interface MyObj : NSObject
@property(nonatomic, assign) NSInteger value;
@property(nonatomic) void *a;
+(void) ofc:(NSInteger)v;
@end

@implementation MyObj
@synthesize value = _value;
@synthesize a = _a;
+(void) ofc:(NSInteger)v {
    
}
@end

@interface ViewController ()
@property(nonatomic, readonly) MyObj *myObj;
@end

@implementation ViewController

@synthesize myObj = _myObj;
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    _myObj = [[MyObj alloc] init];
    _myObj.a = Test::alloc();
}

- (void)viewWillAppear:(BOOL)animated {
    struct ifaddrs* interfaces;
    int error = getifaddrs(&interfaces);
    if (error != 0) {
        NSLog(@"getifaddrs error %d", errno);
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
