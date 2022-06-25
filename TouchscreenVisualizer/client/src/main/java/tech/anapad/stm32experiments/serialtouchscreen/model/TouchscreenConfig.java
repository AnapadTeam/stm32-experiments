package tech.anapad.stm32experiments.serialtouchscreen.model;

/**
 * {@link TouchscreenConfig} represents the touchscreen configuration.
 */
public class TouchscreenConfig {

    private final int xResolution;
    private final int yResolution;

    /**
     * Instantiates a new {@link TouchscreenConfig}.
     *
     * @param xResolution the X resolution
     * @param yResolution the Y resolution
     */
    public TouchscreenConfig(int xResolution, int yResolution) {
        this.xResolution = xResolution;
        this.yResolution = yResolution;
    }

    public int getXResolution() {
        return xResolution;
    }

    public int getYResolution() {
        return yResolution;
    }
}
