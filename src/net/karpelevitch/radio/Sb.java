package net.karpelevitch.radio;

import com.google.common.base.Preconditions;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.stream.Collectors;

import static java.lang.Integer.parseInt;

class Sb extends State<Sb> {
    public ArrayList<Group> groups;
    public final boolean normalized;
    private int count;

    public Sb(String groupStrings, boolean normalized) {
        this.normalized = normalized;
        String[] gr = groupStrings.split(",");
        groups = new ArrayList<>(gr.length);
        for (int i = 0; i < gr.length; i++) {
            Group group = new Group(gr[i]);
            add(group);
            if (normalized && i > 0) {
                if (group.compareTo(groups.get(i - 1)) > 0)
                    throw new RuntimeException(groupStrings + " out of order: " + group + " > " + groups.get(i - 1));
            }
        }
    }

    public void add(Group group) {
        groups.add(group);
        pairs += group.pairs;
        this.count += group.a;
        this.count += group.b;
    }

    public Sb(ArrayList<Group> g, boolean normalized) {
        this.normalized = normalized;
        this.groups = g;
        for (Group group : groups) {
            pairs += group.pairs;
            this.count += group.a;
            this.count += group.b;
        }
    }
    public Sb(long... n) {
        Preconditions.checkArgument(n.length % 2 == 0);
        this.groups = new ArrayList<>(n.length/2);
        for (int i = 0; i < n.length; i+=2) {
            add(Group.of(n[i], n[i + 1]));
        }
        this.normalized = false;
    }

    @Override
    public String toString() {
        return groups.stream().map(Group::toString).collect(Collectors.joining(",", "Sb(", ")[" + pairs + "," + count + "]"));
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Sb sb = (Sb) o;

        return groups.equals(sb.groups);
    }

    @Override
    public int hashCode() {
        return groups.hashCode();
    }

    @Override
    public Sb normalize() {
        if (this.normalized) return this;
        ArrayList<Group> g = new ArrayList<>();
        for (Group group : groups) {
            if (group.pairs > 1) g.add(group); // drop empty and unit groups
        }
        g.sort(Comparator.reverseOrder()); // sort
        return new Sb(g, true);
    }

    @Override
    public boolean isInferiorTo(State s) {
        if (s instanceof Sb) {
            Sb sb = (Sb) s;
            if (!normalized || !sb.normalized) throw new RuntimeException("should only be called for normalized");
            if (sb.groups.size() < groups.size()) return false;
            for (int i = 0; i < groups.size(); i++) {
                if (!groups.get(i).isInferiorTo(sb.groups.get(i))) return false;
            }
            return true;
        }
        return false;
    }

    @Override
    public boolean isTrivial() {
        return normalized?groups.isEmpty(): normalize().isTrivial();
    }

    public void add(long n1, long n2) {
        add(Group.of(n1, n2));
    }

    public static class Group implements Comparable<Group> {
        public static final Group EMPTY = new Group(0, 0);
        public static final Group UNIT = new Group(1, 1);

        public final long a;
        public final long b;
        public final long pairs;

        public Group(long a, long b) {
            if (b > a) throw new RuntimeException("a:" + a + " b: " + b);
            this.a = a;
            this.b = b;
            this.pairs = a * b;
        }

        public Group(String s) {
            this(s.split(":"));
        }

        public Group(String[] s) {
            this(parseInt(s[0]), parseInt(s[1]));
        }

        public static Group of(long n1, long n2) {
            Preconditions.checkArgument(n1>=0);
            Preconditions.checkArgument(n2>=0);
            if (n1==0 || n2==0) return EMPTY;
            if (n1==1 && n2==1) return UNIT;
            if (n1>=n2) return new Group(n1, n2);
            return new Group(n2, n1);
        }

        @Override
        public int compareTo(Group o) {
            if (pairs != o.pairs) return Long.compare(pairs,o.pairs);
            return Long.compare(a,o.a);
        }

        @Override
        public String toString() {
            return "" + a + ":" + b;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;

            Group group = (Group) o;

            if (a != group.a) return false;
            return b == group.b;
        }

        @Override
        public int hashCode() {
            int result = (int) (a ^ (a >>> 32));
            result = 31 * result + (int) (b ^ (b >>> 32));
            return result;
        }

        public boolean isInferiorTo(Group o) {
            return a <= o.a && b <= o.b;
        }
    }
}
