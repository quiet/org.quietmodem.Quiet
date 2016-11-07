package org.quietmodem.Quiet;

public class Complex {
    private float real;
    private float imaginary;

    Complex(float real, float imaginary) {
        this.real = real;
        this.imaginary = imaginary;
    }

    public float getReal() {
        return this.real;
    }

    public float getImaginary() {
        return this.imaginary;
    }

    public void setReal(float real) {
        this.real = real;
    }

    public void setImaginary(float imaginary) {
        this.imaginary = imaginary;
    }
}
