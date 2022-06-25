package tech.anapad.stm32experiments.serialtouchscreen.model;

/**
 * {@link TouchscreenTouch} represents a touchscreen finger touch.
 */
public class TouchscreenTouch {

    private final int x;
    private final int y;
    private final int size;

    /**
     * Instantiates a new {@link TouchscreenTouch}.
     *
     * @param x    the X
     * @param y    the Y
     * @param size the size
     */
    public TouchscreenTouch(int x, int y, int size) {
        this.x = x;
        this.y = y;
        this.size = size;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public int getSize() {
        return size;
    }
}
