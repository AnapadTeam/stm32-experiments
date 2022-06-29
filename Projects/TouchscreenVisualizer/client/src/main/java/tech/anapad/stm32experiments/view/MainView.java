package tech.anapad.stm32experiments.view;

import javafx.geometry.Rectangle2D;
import javafx.stage.Screen;
import javafx.stage.Stage;
import tech.anapad.stm32experiments.TouchscreenVisualizerClient;
import tech.anapad.stm32experiments.view.options.OptionsScene;
import tech.anapad.stm32experiments.view.visualizer.VisualizerScene;

/**
 * {@link MainView} is the main view of the GUI application.
 */
public class MainView {

    private final TouchscreenVisualizerClient touchscreenVisualizer;

    private Stage stage;
    private OptionsScene optionsScene;
    private VisualizerScene visualizerScene;

    /**
     * Instantiates a new {@link MainView}.
     *
     * @param touchscreenVisualizer the {@link TouchscreenVisualizerClient}
     */
    public MainView(TouchscreenVisualizerClient touchscreenVisualizer) {
        this.touchscreenVisualizer = touchscreenVisualizer;
    }

    /**
     * Starts the {@link MainView}. JavaFX objects must be created within this method and not in the constructor.
     *
     * @param primaryStage the primaryStage
     */
    public void start(Stage primaryStage) {
        stage = primaryStage;
        optionsScene = new OptionsScene(this);
        visualizerScene = new VisualizerScene(this);

        setupOptionsScene();
        setupStage();
    }

    private void setupOptionsScene() {
        optionsScene.start();
    }

    private void setupStage() {
        Rectangle2D primaryScreenBounds = Screen.getPrimary().getVisualBounds();
        stage.setX(primaryScreenBounds.getMinX());
        stage.setY(primaryScreenBounds.getMinY());
        stage.setWidth(primaryScreenBounds.getWidth());
        stage.setHeight(primaryScreenBounds.getHeight());
        stage.setMinWidth(500);
        stage.setMinHeight(400);

        stage.setTitle("STM32-Experiments Touchscreen Visualizer");
        stage.setScene(optionsScene.getScene());
        stage.show();
    }

    /**
     * Stops the {@link MainView}.
     */
    public void stop() {
        optionsScene.stop();
        visualizerScene.stop();
    }

    public TouchscreenVisualizerClient getTouchscreenVisualizerClient() {
        return touchscreenVisualizer;
    }

    public Stage getStage() {
        return stage;
    }

    public OptionsScene getOptionsScene() {
        return optionsScene;
    }

    public VisualizerScene getVisualizerScene() {
        return visualizerScene;
    }
}
