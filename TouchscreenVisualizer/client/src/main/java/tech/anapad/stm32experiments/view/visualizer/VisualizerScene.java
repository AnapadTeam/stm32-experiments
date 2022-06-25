package tech.anapad.stm32experiments.view.visualizer;

import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenConfig;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenTouch;
import tech.anapad.stm32experiments.util.exception.ExceptionUtils;
import tech.anapad.stm32experiments.view.MainView;

import java.util.function.Consumer;

/**
 * {@link VisualizerScene} is the scene allowing the user visualize the touchscreen touches.
 */
public class VisualizerScene {

    private final MainView mainView;
    private final Scene scene;
    private final VBox vBox;

    private HBox backButtonHBox;
    private VisualizerCanvas visualizerCanvas;
    private Consumer<TouchscreenConfig> setCanvasTouchscreenConfig;
    private Consumer<TouchscreenTouch[]> setCanvasTouchscreenTouches;

    /**
     * Instantiates a new {@link VisualizerScene}.
     *
     * @param mainView the {@link MainView}
     */
    public VisualizerScene(MainView mainView) {
        this.mainView = mainView;

        setupBackButton();
        setupCanvas();

        vBox = new VBox(backButtonHBox, visualizerCanvas);
        scene = new Scene(vBox);

        postSetupCanvas();
    }

    private void setupBackButton() {
        Button backButton = new Button("Back");
        backButton.addEventHandler(MouseEvent.MOUSE_CLICKED, event -> {
            stop();
            mainView.getStage().setScene(mainView.getOptionsScene().getScene());
            mainView.getOptionsScene().start();
        });

        backButtonHBox = new HBox(backButton);
        backButtonHBox.setAlignment(Pos.CENTER);
        backButtonHBox.setPadding(new Insets(20));
    }

    private void setupCanvas() {
        visualizerCanvas = new VisualizerCanvas();
        setCanvasTouchscreenConfig = visualizerCanvas::setTouchscreenConfig;
        setCanvasTouchscreenTouches = visualizerCanvas::setTouchscreenTouches;
    }

    private void postSetupCanvas() {
        visualizerCanvas.widthProperty().bind(vBox.widthProperty());
        visualizerCanvas.heightProperty().bind(vBox.heightProperty().subtract(backButtonHBox.heightProperty()));
    }

    /**
     * Starts this {@link VisualizerScene}.
     */
    public void start() {
        String serialSystemPortName = mainView.getOptionsScene().getSerialDeviceSelection();

        int baudRate;
        try {
            baudRate = Integer.parseInt(mainView.getOptionsScene().getBaudRateString());
        } catch (Exception exception) {
            ExceptionUtils.showException("Baud rate must be a number!", exception, false);
            return;
        }

        try {
            mainView.getTouchscreenVisualizer().getSerialController().initTouchscreenSerialPort(
                    serialSystemPortName, baudRate, setCanvasTouchscreenConfig, setCanvasTouchscreenTouches);
        } catch (Exception exception) {
            ExceptionUtils.showException("Could not initialize serial port data processing!", exception, false);
        }
    }

    /**
     * Stops this {@link VisualizerScene}.
     */
    public void stop() {
        mainView.getTouchscreenVisualizer().getSerialController().stop();
    }

    public Scene getScene() {
        return scene;
    }
}
