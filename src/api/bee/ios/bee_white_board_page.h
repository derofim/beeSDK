/**
 *  @file        bee_white_board_page.h
 *  @brief       BeeSDK白板页声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_WHITE_BOARD_PAGE_H__
#define __BEE_WHITE_BOARD_PAGE_H__

#import <UIKit/UIKit.h>
#import "bee/ios/bee_white_board_def.h"
#import "bee/ios/bee_white_board_item.h"

/**
 *  @brief  文档加载结果回调Block类型.
 *  @param  image       加载的图像.
 */
typedef void(^DocReadyHandler)(UIImage *image);

/**
 *  @brief  白板页加载结果回调Block类型.
 *  @param  error       加载结果.
 */
typedef void(^LoadHandler)(NSError *error);

/// BeeSDK白板页类.
@interface BeeWhiteBoardPage : NSObject

/// 撤销CALayer栈.
@property(nonatomic, strong) NSMutableArray *undoStack;

/// 重做CALayer栈.
@property(nonatomic, strong) NSMutableArray *redoStack;

/// 白板页背景颜色.
@property(nonatomic, copy) UIColor *backgroundColor;

/// 白板页的所有者/绘制者.
@property(nonatomic, copy) NSString *src;

/// 白板页的文档url.
@property(nonatomic, copy) NSString *url;

/// 白板页画布尺寸.
@property(nonatomic, assign) CGSize canvasSize;

/// 文档参考位置.
@property(nonatomic, assign) CGPoint refPos;

/// 白板页加载的图像.
@property(nonatomic, strong) UIImage *image;

/**
 *  @brief  白板页类构造函数.
 *  @param  src         白板页的所有者/绘制者.
 *  @param  url         白板页的文档url.
 *  @param  canvasSize  白板页画布尺寸.
 *  @param  refPos      文档参考位置.
 *  @param  targetSize  本地显示的尺寸.
 *  @return 白板页类对象.
 */
- (instancetype)initWithParam:(NSString*)src
                          url:(NSString*)url
                   canvasSize:(CGSize)canvasSize
                       refPos:(CGPoint)refPos
                   targetSize:(CGSize)targetSize;

/**
 *  @brief  加载白板页的所有元素.
 *  @param  docHandler  白板页文档加载结果回调Block.
 *  @param  loadHandler 白板页所有元素加载结果回调Block.
 *  @note   先获得文档的加载结果可以在绘制白板页元素前设置一些属性，例如使用图片设置橡皮擦的颜色.
 */
- (void)load:(DocReadyHandler)docHandler loadHandler:(LoadHandler)loadHandler;

/**
 *  @brief  缓存一个撤销CALayer.
 *  @param  layer       撤销CALayer.
 */
- (void)pushUndo:(CALayer *)layer;

/**
 *  @brief  取出最近的一个撤销CALayer.
 *  @return 最近的一个撤销CALayer.
 */
- (CALayer*)popUndo;

/**
 *  @brief  缓存一个重做CALayer.
 *  @param  layer       重做CALayer.
 */
- (void)pushRedo:(CALayer *)layer;

/**
 *  @brief  取出最近的一个重做CALayer.
 *  @return 最近的一个重做CALayer.
 */
- (CALayer*)popRedo;

/**
 *  @brief  清除撤销CALayer栈.
 */
- (void)clearUndoStack;

/**
 *  @brief  清除重做CALayer栈.
 */
- (void)clearRedoStack;

/**
 *  @brief  缓存一个元素到指定的CALayer栈.
 *  @param  item        白板页元素.
 *  @param  stack       CALayer栈，撤销、重做.
 */
- (void)cacheItem:(BeeWhiteBoardItem*)item stack:(BeeWhiteBoardStack)stack;

/**
 *  @brief  缓存绘制者到CALayer的关系，用于画线.
 *  @param  local       本地绘制还是从网络绘制.
 *  @param  drawer      绘制者.
 *  @param  layer       用于显示的CALayer.
 */
- (void)refLayer:(BOOL)local drawer:(NSString*)drawer layer:(CALayer*)layer;

/**
 *  @brief  获取某个绘制者正在绘制的CALayer，用于画线.
 *  @param  local       本地绘制还是从网络绘制.
 *  @param  drawer      绘制者.
 *  @return 用于显示的CALayer.
 */
- (CALayer*)getLayer:(BOOL)local drawer:(NSString*)drawer;

/**
 *  @brief  删除绘制者到CALayer的关系.
 *  @param  local       本地绘制还是从网络绘制.
 *  @param  drawer      绘制者.
 */
- (void)unrefLayer:(BOOL)local drawer:(NSString*)drawer;

@end

#endif // #ifndef __BEE_WHITE_BOARD_PAGE_H__
