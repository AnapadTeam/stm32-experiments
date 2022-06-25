package tech.anapad.stm32experiments;

import javafx.application.Application;
import javafx.stage.Stage;
import tech.anapad.stm32experiments.serialtouchscreen.SerialTouchscreenDeviceController;
import tech.anapad.stm32experiments.util.exception.ExceptionUtils;
import tech.anapad.stm32experiments.view.MainView;

/**
 * {@link TouchscreenVisualizer} is the root object of this application.
 */
public class TouchscreenVisualizer extends Application {

    private SerialTouchscreenDeviceController serialTouchscreenDeviceController;
    private MainView mainView;

    @Override
    public void init() {
        serialTouchscreenDeviceController = new SerialTouchscreenDeviceController();
        mainView = new MainView(this);
        ExceptionUtils.setMainViewInstance(mainView);
    }

    @Override
    public void start(Stage primaryStage) {
        mainView.start(primaryStage);
    }

    @Override
    public void stop() {
        mainView.stop();
        serialTouchscreenDeviceController.stop();
    }

    public SerialTouchscreenDeviceController getSerialController() {
        return serialTouchscreenDeviceController;
    }

    public MainView getMainView() {
        return mainView;
    }

    /**
     * The entry point of application.
     *
     * @param args the input arguments
     */
    public static void main(String[] args) {
        launch(args);
    }
}
