package tech.anapad.stm32experiments.view.visualizer;

import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenConfig;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenTouch;

/**
 * {@link VisualizerCanvas} is the {@link Canvas} for the {@link VisualizerScene}.
 */
public class VisualizerCanvas extends Canvas {

    private static final double MARGIN = 25;

    private final GraphicsContext graphicsContext;

    private TouchscreenConfig touchscreenConfig;
    private TouchscreenTouch[] touchscreenTouches;

    /**
     * Instantiates a new {@link VisualizerCanvas}.
     */
    public VisualizerCanvas() {
        graphicsContext = getGraphicsContext2D();

        this.widthProperty().addListener(observable -> paint());
        this.heightProperty().addListener(observable -> paint());
    }

    /**
     * Paint the {@link VisualizerCanvas}. Must be called on the JavaFX thread.
     */
    public void paint() {
        graphicsContext.clearRect(0, 0, getWidth(), getHeight());

        if (touchscreenConfig == null || touchscreenTouches == null) {
            return;
        }

        double widthWithMargin = getWidth() - (MARGIN * 2);
        double heightWithMargin = getHeight() - (MARGIN * 2);

        // The following is basically calculating the "background-size: contain" CSS equivalent for this canvas
        double canvasAspectRatio = widthWithMargin / heightWithMargin;
        double touchscreenAspectRatio =
                (double) touchscreenConfig.getXResolution() / (double) touchscreenConfig.getYResolution();
        double drawWidth = widthWithMargin;
        double drawHeight = heightWithMargin;
        if (touchscreenAspectRatio > canvasAspectRatio) { // Clamp top and bottom
            drawHeight = widthWithMargin / touchscreenAspectRatio;
        } else { // Clamp left and right
            drawWidth = heightWithMargin * touchscreenAspectRatio;
        }
        double drawX = (widthWithMargin / 2) - (drawWidth / 2) + MARGIN;
        double drawY = (heightWithMargin / 2) - (drawHeight / 2) + MARGIN;
        double drawArea = drawWidth * drawHeight;

        // Draw border
        graphicsContext.setStroke(Color.BLACK);
        graphicsContext.setLineWidth(drawArea * 0.000007);
        double widthHeightAverage = (drawWidth + drawHeight) / 2;
        double arcSize = widthHeightAverage * 0.05;
        graphicsContext.strokeRoundRect(drawX, drawY, drawWidth, drawHeight, arcSize, arcSize);

        // Draw touches
        graphicsContext.setFill(Color.CORNFLOWERBLUE);
        double xMultiplier = drawWidth / touchscreenConfig.getXResolution();
        double yMultiplier = drawHeight / touchscreenConfig.getYResolution();
        for (TouchscreenTouch touchscreenTouch : touchscreenTouches) {
            graphicsContext.fillOval(
                    drawX + (double) touchscreenTouch.getX() * xMultiplier,
                    drawY + (double) touchscreenTouch.getY() * yMultiplier,
                    (double) touchscreenTouch.getSize() * xMultiplier,
                    (double) touchscreenTouch.getSize() * yMultiplier);
        }
    }

    @Override
    public boolean isResizable() {
        return true;
    }

    public void setTouchscreenConfig(TouchscreenConfig touchscreenConfig) {
        this.touchscreenConfig = touchscreenConfig;
    }

    public void setTouchscreenTouches(TouchscreenTouch[] touchscreenTouches) {
        this.touchscreenTouches = touchscreenTouches;
    }
}
