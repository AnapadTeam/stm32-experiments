package tech.anapad.stm32experiments.serialtouchscreen.model;

/**
 * {@link TouchscreenTouch} represents a touchscreen finger touch.
 */
public class TouchscreenTouch {

    private final int id;
    private final int x;
    private final int y;
    private final int size;

    /**
     * Instantiates a new {@link TouchscreenTouch}.
     *
     * @param id   the ID
     * @param x    the X
     * @param y    the Y
     * @param size the size
     */
    public TouchscreenTouch(int id, int x, int y, int size) {
        this.id = id;
        this.x = x;
        this.y = y;
        this.size = size;
    }

    public int getID() {
        return id;
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

    @Override
    public String toString() {
        return "TouchscreenTouch{" +
                "id=" + id +
                ", x=" + x +
                ", y=" + y +
                ", size=" + size +
                '}';
    }
}
