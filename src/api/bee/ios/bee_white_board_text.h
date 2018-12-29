/**
 *  @file        bee_white_board_text.h
 *  @brief       BeeSDK白板文本声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_WHITE_BOARD_TEXT_H__
#define __BEE_WHITE_BOARD_TEXT_H__

#import <UIKit/UIKit.h>
#import "bee/ios/bee_white_board_item.h"

using namespace bee;

/// BeeSDK白板文本类.
@interface BeeWhiteBoardText : BeeWhiteBoardItem

/// 文本内容.
@property(nonatomic, copy) NSString *data;

/// 显示位置.
@property(nonatomic, assign) CGPoint pos;

/**
 *  @brief  白板文本类构造函数.
 *  @param  fontColor       文本颜色.
 *  @param  fontSize        文本字号.
 *  @param  mode            绘制模式.
 *  @param  data            文本内容.
 *  @param  pos             显示位置.
 *  @return 白板文本类对象.
 */
- (instancetype)initWithParam:(CGColorRef)fontColor
                     fontSize:(NSInteger)fontSize
                         mode:(BeeDrawingMode)mode
                         data:(NSString*)data
                          pos:(CGPoint)pos;

@end

#endif // #ifndef __BEE_WHITE_BOARD_TEXT_H__
