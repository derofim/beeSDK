/**
 *  @file        bee_white_board_line.h
 *  @brief       BeeSDK白板线声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_WHITE_BOARD_LINE_H__
#define __BEE_WHITE_BOARD_LINE_H__

#import <UIKit/UIKit.h>
#import "bee/ios/bee_white_board_item.h"

using namespace bee;

/// BeeSDK白板线类.
@interface BeeWhiteBoardLine : BeeWhiteBoardItem

/// 组成线的点数组.
@property(nonatomic, strong) NSArray *path;

/**
 *  @brief  白板线类构造函数.
 *  @param  strokeColor     线的颜色.
 *  @param  strokeWidth     线的粗细.
 *  @param  mode            绘制模式，可以是画笔或者橡皮.
 *  @param  path            组成线的点数组.
 *  @return 白板线类对象.
 */
- (instancetype)initWithParam:(CGColorRef)strokeColor
                  strokeWidth:(NSInteger)strokeWidth
                         mode:(BeeDrawingMode)mode
                         path:(NSArray*)path;

@end

#endif // #ifndef __BEE_WHITE_BOARD_LINE_H__
