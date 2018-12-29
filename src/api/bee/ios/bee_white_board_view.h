/**
 *  @file        bee_white_board_view.h
 *  @brief       BeeSDK白板View声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_WHITE_BOARD_VIEW_H__
#define __BEE_WHITE_BOARD_VIEW_H__

#import <UIKit/UIKit.h>
#import "bee/ios/bee_white_board_def.h"

using namespace bee;

/// BeeSDK白板View委托协议，可以获得本地绘制的轨迹信息.
@protocol BeeWhiteBoardViewDelegate <NSObject>

/**
 *  @brief  本地绘制一条线段的通知.
 *  @param  begin       线段起始点.
 *  @param  end         线段终点.
 *  @param  color       线段的颜色.
 *  @param  width       线段的宽度.
 *  @param  mode        绘制模式.
 *  @note   一条曲线由若干连续的线段组成.
 */
- (void)onDrawingLine:(CGPoint)begin
                  end:(CGPoint)end
                color:(CGColorRef)color
                width:(CGFloat)width
                 mode:(BeeDrawingMode)mode;

/**
 *  @brief  绘制当前曲线结束的通知.
 */
- (void)onDrawLineEnd;

@end

@class BeeWhiteBoardPage;

/// BeeSDK白板View类.
@interface BeeWhiteBoardView : UIView

/**
 * @brief   白板View构造函数.
 * @param   frame       显示区域.
 * @return  白板View对象.
 */
- (instancetype)initWithFrame:(CGRect)frame;

/**
 * @brief   设置白板View委托，以获取本地绘制的通知.
 * @param   delegate    白板View委托.
 */
- (void)setWhiteBoardDelegate:(id<BeeWhiteBoardViewDelegate>)delegate;

/**
 * @brief   设置白板页信息并从指定的页加载.
 * @param   pages               白板页数组.
 * @param   currentPageIndex    当前页码.
 * @param   canvasSize          画布尺寸.
 * @param   refPos              文档参考位置.
 */
- (void)setBoardInfo:(NSArray<BeeWhiteBoardPage*>*)pages
    currentPageIndex:(NSInteger)currentPageIndex
          canvasSize:(CGSize)canvasSize
              refPos:(CGPoint)refPos;

/**
 * @brief   加载并显示下一页.
 */
- (void)nextPage;

/**
 * @brief   加载并显示上一页.
 */
- (void)prePage;

/**
 * @brief   清除当前页的屏幕.
 */
- (void)clearScreen;

/**
 * @brief   取消上一次绘制.
 */
- (void)undo;

/**
 * @brief   重做上一次取消的绘制.
 */
- (void)redo;

/**
 * @brief   开始画一条曲线，一条曲线由若干连续的线段组成.
 * @param   point       曲线起始点.
 * @param   strokeColor 线的颜色.
 * @param   strokeWidth 线的宽度.
 * @param   mode        绘制模式.
 * @param   drawer      绘制者标识.
 */
- (void)lineBegin:(CGPoint)point
      strokeColor:(CGColorRef)strokeColor
      strokeWidth:(CGFloat)strokeWidth
             mode:(BeeDrawingMode)mode
           drawer:(NSString*)drawer;

/**
 * @brief   绘制到当前点，表示从上一个点到当前点绘制了一条线段.
 * @param   point       当前点.
 * @param   mode        绘制模式.
 * @param   drawer      绘制者.
 */
- (void)lineMove:(CGPoint)point
            mode:(BeeDrawingMode)mode
          drawer:(NSString*)drawer;

/**
 * @brief   绘制当前曲线结束.
 * @param   drawer      绘制者.
 */
- (void)lineEnd:(NSString*)drawer;

/**
 * @brief   绘制一个文本.
 * @param   text        文本内容.
 * @param   point       显示位置.
 * @param   color       文本颜色.
 * @param   size        文本字号.
 * @param   drawer      绘制者.
 */
- (void)drawText:(NSString*)text
             pos:(CGPoint)point
           color:(CGColorRef)color
            size:(CGFloat)size
          drawer:(NSString*)drawer;

/**
 * @brief   设置本地绘制模式.
 * @param   mode        绘制模式.
 */
- (void)setDrawingMode:(BeeDrawingMode)mode;

/**
 * @brief   获取本地绘制模式.
 * @return  本地绘制模式.
 */
- (BeeDrawingMode)getDrawingMode;

/**
 * @brief   设置本地线条绘制颜色.
 * @param   rgbColor    线条颜色.
 */
- (void)setLineColor:(int)rgbColor;

/**
 * @brief   获取本地线条绘制颜色.
 * @return  本地线条绘制颜色.
 */
- (UIColor*)getLineColor;

/**
 * @brief   设置本地线条绘制宽度.
 * @param   width       线条宽度.
 */
- (void)setLineWidth:(CGFloat)width;

/**
 * @brief   获取本地线条绘制宽度.
 * @return  本地线条绘制宽度.
 */
- (CGFloat)getLineWidth;

/**
 * @brief   关闭绘制，默认为打开.
 */
- (void)lockDrawing;

/**
 * @brief   打开绘制，默认为打开.
 */
- (void)unlockDrawing;

/**
 * @brief   重置白板View的显示位置为初始位置.
 */
- (void)resetFrame;

/**
 * @brief   播放一个视频.
 * @param   url       视频url.
 * @param   pos       视频显示位置.
 * @param   size      视频显示大小.
 */
- (void)playVideo:(NSString*)url
              pos:(CGPoint)pos
             size:(CGSize)size;

/**
 * @brief   停止播放当前视频.
 */
- (void)stopVideo;

/**
 * @brief   暂停播放当前视频.
 */
- (void)pauseVideo;

/**
 * @brief   继续播放当前视频.
 */
- (void)resumeVideo;

@end

#endif // #ifndef __BEE_WHITE_BOARD_VIEW_H__
