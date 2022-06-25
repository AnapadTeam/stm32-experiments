package tech.anapad.stm32experiments.view.options;

import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.ColumnConstraints;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.StackPane;
import org.controlsfx.control.SearchableComboBox;
import tech.anapad.stm32experiments.util.view.BoxUtils;
import tech.anapad.stm32experiments.view.MainView;

import java.util.Timer;
import java.util.TimerTask;

/**
 * {@link OptionsScene} is the scene allowing the user to configure various options.
 */
public class OptionsScene {

    private final MainView mainView;
    private final Scene scene;
    private final GridPane gridpane;

    private Label serialDeviceSelectionLabel;
    private SearchableComboBox<String> serialDevicesComboBox;
    private Label baudRateLabel;
    private TextField baudRateTextField;
    private Button visualizeTouchscreenButton;
    private Timer serialDevicePollerTimer;
    private TimerTask serialDevicePollerTimerTask;

    /**
     * Instantiates a new {@link OptionsScene}.
     *
     * @param mainView the {@link MainView}
     */
    public OptionsScene(MainView mainView) {
        this.mainView = mainView;

        gridpane = new GridPane();
        setupGridPane();
        setupSerialDeviceSelection();
        setupBaudRateInput();
        setupVisualizeButton();
        setupNodes();

        scene = new Scene(new StackPane(gridpane));
    }

    private void setupGridPane() {
        // Column constraints
        ColumnConstraints firstColumn = new ColumnConstraints();
        firstColumn.setPercentWidth(50);
        firstColumn.setFillWidth(true);
        ColumnConstraints secondColumn = new ColumnConstraints();
        secondColumn.setPercentWidth(50);
        gridpane.getColumnConstraints().addAll(firstColumn, secondColumn);

        gridpane.setVgap(20);

        gridpane.setMinSize(300, 400);
        gridpane.setPrefSize(300, 400);
        gridpane.setMaxSize(300, 400);
    }

    private void setupSerialDeviceSelection() {
        serialDeviceSelectionLabel = new Label("Select serial device:");

        serialDevicesComboBox = new SearchableComboBox<>();
        serialDevicesComboBox.setEditable(false);
        serialDevicesComboBox.setPrefWidth(150);
    }

    private void setupBaudRateInput() {
        baudRateLabel = new Label("Baud Rate:");
        baudRateTextField = new TextField("115200");
    }

    private void setupVisualizeButton() {
        visualizeTouchscreenButton = new Button("Visualize Touchscreen!");
        visualizeTouchscreenButton.addEventHandler(MouseEvent.MOUSE_CLICKED, event -> {
            stop();
            mainView.getStage().setScene(mainView.getVisualizerScene().getScene());
            mainView.getVisualizerScene().start();
        });
    }

    private void setupNodes() {
        gridpane.add(new HBox(
                        serialDeviceSelectionLabel,
                        BoxUtils.getHBoxSpacer(),
                        serialDevicesComboBox),
                0, gridpane.getRowCount(), gridpane.getColumnCount(), 1);
        gridpane.add(new HBox(
                        baudRateLabel,
                        BoxUtils.getHBoxSpacer(),
                        baudRateTextField),
                0, gridpane.getRowCount(), gridpane.getColumnCount(), 1);
        gridpane.add(new HBox(
                        BoxUtils.getHBoxSpacer(),
                        visualizeTouchscreenButton,
                        BoxUtils.getHBoxSpacer()),
                0, gridpane.getRowCount(), gridpane.getColumnCount(), 1);
    }

    /**
     * Starts this {@link OptionsScene}.
     */
    public void start() {
        serialDevicePollerTimer = new Timer("Serial Device Poller Timer");
        serialDevicePollerTimerTask = new TimerTask() {
            @Override
            public void run() {
                Platform.runLater(() -> serialDevicesComboBox.setItems(FXCollections.observableArrayList(
                        mainView.getTouchscreenVisualizer().getSerialController().getAvailableSerialPorts())));
            }
        };
        serialDevicePollerTimer.schedule(serialDevicePollerTimerTask, 500, 2500);
    }

    /**
     * Stops this {@link OptionsScene}.
     */
    public void stop() {
        if (serialDevicePollerTimerTask != null) {
            serialDevicePollerTimerTask.cancel();
        }
        if (serialDevicePollerTimer != null) {
            serialDevicePollerTimer.cancel();
        }
    }

    /**
     * Gets the user's serial device selection.
     *
     * @return the serial device port name
     */
    public String getSerialDeviceSelection() {
        return serialDevicesComboBox.getSelectionModel().getSelectedItem();
    }

    /**
     * Gets the baud rate user input.
     *
     * @return the baud rate user input
     */
    public String getBaudRateString() {
        return baudRateTextField.getText();
    }

    public Scene getScene() {
        return scene;
    }
}
