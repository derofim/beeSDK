//
//  ViewController.m
//  TestOpenssl
//
//  Created by 孙磊 on 2018/2/7.
//  Copyright © 2018年 sohu. All rights reserved.
//

#import "ViewController.h"
#import "TestOpenssl.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    TestOpenSSL t;
    t.test();
    // Do any additional setup after loading the view, typically from a nib.
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
