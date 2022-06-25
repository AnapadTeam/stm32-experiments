module TouchscreenVisualizer {
    requires javafx.base;
    requires javafx.controls;
    requires javafx.graphics;
    exports tech.anapad.stm32experiments to javafx.graphics;

    requires org.controlsfx.controls;

    requires com.fazecast.jSerialComm;
}
