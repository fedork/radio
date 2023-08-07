package net.karpelevitch.radio;

class Sa extends State<Sa> {
    public final int count;

    Sa(int count) {
        this.count = count;
        this.pairs = count * (count - 1) / 2;
    }

    @Override
    public Sa normalize() {
        return this;
    }

    @Override
    public boolean isInferiorTo(State s) {
        if (s instanceof Sa) {
            Sa sa = (Sa) s;
            return this.count <= sa.count;
        }
        return false;
    }

    @Override
    public boolean isTrivial() {
        return count <= 2;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Sa sa = (Sa) o;

        return count == sa.count;
    }

    @Override
    public int hashCode() {
        return count;
    }

    @Override
    public String toString() {
        return "Sa(" + count + ")[" + pairs + "," + count + "]";
    }
}
