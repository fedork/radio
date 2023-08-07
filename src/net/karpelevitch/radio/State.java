package net.karpelevitch.radio;

public abstract class State<T extends State> {
    protected int pairs;

    public abstract T normalize();

    public abstract boolean isInferiorTo(State s);

    public abstract boolean isTrivial();
}
