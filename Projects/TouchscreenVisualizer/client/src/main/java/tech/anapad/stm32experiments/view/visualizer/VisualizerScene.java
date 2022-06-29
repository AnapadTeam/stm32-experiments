package tech.anapad.stm32experiments.view.visualizer;

import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import org.controlsfx.control.ToggleSwitch;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenConfig;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenTouch;
import tech.anapad.stm32experiments.util.exception.ExceptionUtils;
import tech.anapad.stm32experiments.util.view.BoxUtils;
import tech.anapad.stm32experiments.view.MainView;

import java.util.Timer;
import java.util.TimerTask;
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
    private Label resolutionNumberLabel;
    private Label updateFrequencyNumberLabel;
    private Timer updateFrequencyLabelTimer;
    private TimerTask updateFrequencyLabelTimerTask;
    private long lastTouchUpdateTimestamp;
    private double touchUpdateFrequency;
    private HBox infoLabelsHBox;
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
        setupInfoLabelsHBox();
        setupCanvas();

        vBox = new VBox(controlButtonsHBox, infoLabelsHBox, visualizerCanvas);
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

    private void setupInfoLabelsHBox() {
        Label resolutionLabel = new Label("Resolution: ");
        resolutionNumberLabel = new Label("N/A");

        Label updateFrequencyLabel = new Label("Update Frequency: ");
        updateFrequencyNumberLabel = new Label("0");
        Label updateFrequencyUnitLabel = new Label(" Hz");

        infoLabelsHBox = new HBox(resolutionLabel, resolutionNumberLabel,
                BoxUtils.getHBoxSpacer(75),
                updateFrequencyLabel, updateFrequencyNumberLabel, updateFrequencyUnitLabel);
        infoLabelsHBox.setAlignment(Pos.CENTER);
        infoLabelsHBox.setPadding(new Insets(10));

        lastTouchUpdateTimestamp = System.nanoTime();
        touchUpdateFrequency = 0.0;
    }

    private void setupCanvas() {
        visualizerCanvas = new VisualizerCanvas();
        setCanvasTouchscreenConfig = (touchscreenConfig) -> {
            visualizerCanvas.setTouchscreenConfig(touchscreenConfig);
            Platform.runLater(() -> {
                visualizerCanvas.paint();
                resolutionNumberLabel.setText(touchscreenConfig.getXResolution() +
                        "\u00D7" + touchscreenConfig.getYResolution());
            });
        };
        setCanvasTouchscreenTouches = (touchscreenTouches) -> {
            visualizerCanvas.reportTouchscreenTouches(touchscreenTouches);
            Platform.runLater(visualizerCanvas::paint);

            touchUpdateFrequency = 1.0 / ((System.nanoTime() - lastTouchUpdateTimestamp) / 1E9D);
            lastTouchUpdateTimestamp = System.nanoTime();
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

        updateFrequencyLabelTimer = new Timer("Update Frequency Label Timer");
        updateFrequencyLabelTimerTask = new TimerTask() {
            @Override
            public void run() {
                Platform.runLater(
                        () -> updateFrequencyNumberLabel.setText(String.format("%.0f", touchUpdateFrequency)));
            }
        };
        updateFrequencyLabelTimer.schedule(updateFrequencyLabelTimerTask, 0, 500);
    }

    /**
     * Stops this {@link VisualizerScene}.
     */
    public void stop() {
        if (updateFrequencyLabelTimerTask != null) {
            updateFrequencyLabelTimerTask.cancel();
        }
        if (updateFrequencyLabelTimer != null) {
            updateFrequencyLabelTimer.cancel();
        }
        mainView.getTouchscreenVisualizerClient().getSerialController().stop();
    }

    public Scene getScene() {
        return scene;
    }
}
