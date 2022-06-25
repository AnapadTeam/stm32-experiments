package tech.anapad.stm32experiments.util.exception;

import javafx.application.Platform;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import javafx.stage.Modality;
import tech.anapad.stm32experiments.view.MainView;

/**
 * {@link ExceptionUtils} is a utility class to handle exceptions.
 */
public final class ExceptionUtils {

    private static MainView MAIN_VIEW_INSTANCE;

    /**
     * Sets {@link #MAIN_VIEW_INSTANCE}.
     *
     * @param mainView the {@link MainView}
     */
    public static void setMainViewInstance(MainView mainView) {
        MAIN_VIEW_INSTANCE = mainView;
    }

    /**
     * Shows an exception.
     * <br>
     * This will attempt to show a stacktrace dialog to the user.
     *
     * @param message        the message
     * @param throwable      the {@link Throwable}
     * @param showStacktrace <code>true</code> to show the stacktrace, <code>false</code> to only show the stacktrace
     *                       error message
     */
    public static void showException(String message, Throwable throwable, boolean showStacktrace) {
        if (MAIN_VIEW_INSTANCE != null) {
            Platform.runLater(() -> {
                Alert exceptionAlert = new Alert(Alert.AlertType.ERROR);
                exceptionAlert.setTitle("Error");
                exceptionAlert.initOwner(MAIN_VIEW_INSTANCE.getStage());
                exceptionAlert.initModality(Modality.WINDOW_MODAL);
                exceptionAlert.setResizable(true);
                exceptionAlert.getButtonTypes().add(ButtonType.CLOSE);
                exceptionAlert.getButtonTypes().remove(ButtonType.OK);
                exceptionAlert.setHeaderText(message);
                if (throwable != null) {
                    exceptionAlert.setContentText(showStacktrace ?
                            formatStacktrace(throwable) : throwable.getMessage());
                }
                exceptionAlert.show();
            });
        }

        if (message != null) {
            System.err.println("Error: " + message);
        }
        if (throwable != null) {
            throwable.printStackTrace();
        }
    }

    /**
     * Shows an exception.
     * <br>
     * This will attempt to show a stacktrace dialog to the user.
     *
     * @param throwable the {@link Throwable}
     */
    public static void showException(Throwable throwable) {
        showException(null, throwable, true);
    }

    /**
     * Shows an exception.
     * <br>
     * This will attempt to show a stacktrace dialog to the user.
     *
     * @param message the message
     */
    public static void showException(String message) {
        showException(message, null, false);
    }

    /**
     * Formats a stacktrace with less clutter and only relevant elements.
     *
     * @param throwable the {@link Throwable}
     *
     * @return a {@link String}
     */
    public static String formatStacktrace(Throwable throwable) {
        try {
            StringBuilder formattedString = new StringBuilder();

            formattedString.append(throwable.toString());
            formattedString.append(System.lineSeparator());

            for (StackTraceElement stackTraceElement : throwable.getStackTrace()) {
                formattedString.append("    ");
                formattedString.append(stackTraceElement);
                formattedString.append(System.lineSeparator());
            }

            return formattedString.toString();
        } catch (Exception ignored) {
            return throwable.toString();
        }
    }
}
