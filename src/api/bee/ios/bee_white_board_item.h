/**
 *  @file        bee_white_board_item.h
 *  @brief       BeeSDK白板元素声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_WHITE_BOARD_ITEM_H__
#define __BEE_WHITE_BOARD_ITEM_H__

#import <UIKit/UIKit.h>
#import "bee/ios/bee_white_board_def.h"

using namespace bee;

/// BeeSDK白板元素类.
@interface BeeWhiteBoardItem : NSObject

/// 颜色.
@property(nonatomic, assign) CGColorRef color;

/// 宽度/尺寸.
@property(nonatomic, assign) NSInteger width;

/// 绘制模式.
@property(nonatomic, assign) BeeDrawingMode mode;

/**
 *  @brief  白板元素类构造函数.
 *  @param  color       颜色.
 *  @param  width       宽度/尺寸.
 *  @param  mode        绘制模式.
 *  @return 白板元素类对象.
 */
- (instancetype)initWithParam:(CGColorRef)color
                        width:(NSInteger)width
                         mode:(BeeDrawingMode)mode;

@end

#endif // #ifndef __BEE_WHITE_BOARD_ITEM_H__
