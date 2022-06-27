package tech.anapad.stm32experiments.view.visualizer;

import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import org.controlsfx.control.ToggleSwitch;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenConfig;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenTouch;
import tech.anapad.stm32experiments.util.exception.ExceptionUtils;
import tech.anapad.stm32experiments.util.view.BoxUtils;
import tech.anapad.stm32experiments.view.MainView;

import java.util.function.Consumer;

/**
 * {@link VisualizerScene} is the scene allowing the user visualize the touchscreen touches.
 */
public class VisualizerScene {

    private final MainView mainView;
    private final Scene scene;
    private final VBox vBox;

    private ToggleSwitch drawModeToggle;
    private Button clearDrawingsButton;
    private HBox controlButtonsHBox;
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

        setupControlButtons();
        setupCanvas();

        vBox = new VBox(controlButtonsHBox, visualizerCanvas);
        scene = new Scene(vBox);

        postSetupControlButtons();
        postSetupCanvas();
    }

    private void setupControlButtons() {
        Button backButton = new Button("Back");
        backButton.addEventHandler(MouseEvent.MOUSE_CLICKED, event -> {
            stop();
            mainView.getStage().setScene(mainView.getOptionsScene().getScene());
            mainView.getOptionsScene().start();
        });

        drawModeToggle = new ToggleSwitch("Draw Mode");
        drawModeToggle.setPadding(new Insets(5));
        clearDrawingsButton = new Button("Clear Drawings");
        clearDrawingsButton.setPadding(new Insets(5));

        controlButtonsHBox = new HBox(backButton, BoxUtils.getHBoxSpacer(50), drawModeToggle, clearDrawingsButton);
        controlButtonsHBox.setAlignment(Pos.CENTER);
        controlButtonsHBox.setPadding(new Insets(10));
    }

    private void postSetupControlButtons() {
        visualizerCanvas.drawModeProperty().bind(drawModeToggle.selectedProperty());
        clearDrawingsButton.addEventHandler(MouseEvent.MOUSE_CLICKED,
                event -> visualizerCanvas.clearTouchscreenTouches());
    }

    private void setupCanvas() {
        visualizerCanvas = new VisualizerCanvas();
        setCanvasTouchscreenConfig = visualizerCanvas::setTouchscreenConfig;
        setCanvasTouchscreenTouches = (touchscreenTouches) -> {
            visualizerCanvas.reportTouchscreenTouches(touchscreenTouches);
            Platform.runLater(visualizerCanvas::paint);
        };
    }

    private void postSetupCanvas() {
        visualizerCanvas.widthProperty().bind(vBox.widthProperty());
        visualizerCanvas.heightProperty().bind(vBox.heightProperty()
                .subtract(controlButtonsHBox.heightProperty()));
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
            mainView.getTouchscreenVisualizerClient().getSerialController().initTouchscreenSerialPort(
                    serialSystemPortName, baudRate, setCanvasTouchscreenConfig, setCanvasTouchscreenTouches);
        } catch (Exception exception) {
            ExceptionUtils.showException("Could not initialize serial port data processing!", exception, false);
            return;
        }
    }

    /**
     * Stops this {@link VisualizerScene}.
     */
    public void stop() {
        mainView.getTouchscreenVisualizerClient().getSerialController().stop();
    }

    public Scene getScene() {
        return scene;
    }
}
