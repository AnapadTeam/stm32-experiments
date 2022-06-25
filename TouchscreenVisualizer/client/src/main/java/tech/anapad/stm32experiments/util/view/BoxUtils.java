package tech.anapad.stm32experiments.util.view;

import javafx.geometry.Insets;
import javafx.scene.Node;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.Region;
import javafx.scene.layout.VBox;

/**
 * {@link BoxUtils} is a utility class for {@link HBox}s and {@link VBox}s.
 */
public final class BoxUtils {

    /**
     * Creates a {@link HBox} spacer {@link Node} with h-grow {@link Priority} of {@link Priority#ALWAYS}.
     *
     * @return a spacer {@link Node}
     */
    public static Node getHBoxSpacer() {
        final Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        return spacer;
    }

    /**
     * Creates a {@link HBox} spacer {@link Node} with margin of <code>width</code>.
     *
     * @param width the margin width
     *
     * @return a spacer {@link Node}
     */
    public static Node getHBoxSpacer(double width) {
        final Region spacer = new Region();
        HBox.setMargin(spacer, new Insets(0, width, 0, 0));
        return spacer;
    }

    /**
     * Creates a {@link VBox} spacer {@link Node} with v-grow {@link Priority} of {@link Priority#ALWAYS}.
     *
     * @return a spacer {@link Node}
     */
    public static Node getVBoxSpacer() {
        final Region spacer = new Region();
        VBox.setVgrow(spacer, Priority.ALWAYS);
        return spacer;
    }
}
