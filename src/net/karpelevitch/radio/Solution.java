package net.karpelevitch.radio;

import java.util.Map;

class Solution {

    public static final Solution TRIVIAL = new Solution(null, 0 , null, null, null, null);

    public final State init;
    public final int k;
    public final Operator op;
    public final State b2;
    public final State b1;
    public final State b0;
    public int line;

    public Solution(State init, int k, Operator o, State b2, State b1, State b0) {
        this.init = init;
        this.k = k;
        this.op = o;
        this.b2 = b2;
        this.b1 = b1;
        this.b0 = b0;
    }

    protected Solution getSolution(State st) {
        if (st.isTrivial()) return null;
        Map<State, Solution> solMap = ParseAndPrintSolutions.solutions.get(k - 1);
        // find listed first
        for (Solution s1 : ParseAndPrintSolutions.s) {
            if (s1.k == k - 1 && st.isInferiorTo(s1.init)) {
                solMap.put(st, s1);
                return s1;
            }
        }
        Solution solution = solMap.get(st);
        if (solution != null) {
            return solution;
        }
        for (Map.Entry<State, Solution> entry : solMap.entrySet()) {
            if (st.isInferiorTo(entry.getKey())) {
                solMap.put(st, entry.getValue());
                return entry.getValue();
            }
        }
        throw new RuntimeException("could not find solution for " + st);
    }

    public Solution getS2() {
        return getSolution(this.b2.normalize());
    }

    public Solution getS1() {
        return getSolution(this.b1.normalize());
    }

    public Solution getS0() {
        return getSolution(this.b0.normalize());
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Solution solution = (Solution) o;

        if (k != solution.k) return false;
        return init.equals(solution.init);
    }

    @Override
    public int hashCode() {
        int result = init.hashCode();
        result = 31 * result + k;
        return result;
    }

    @Override
    public String toString() {
        return "Solution{" +
                "init=" + init +
                ", k=" + k +
                ", op=" + op +
                ", b2=" + b2 +
                ", b1=" + b1 +
                ", b0=" + b0 +
                ", line=" + line +
//                ", s2=" + getS2() +
//                ", s1=" + getS1() +
//                ", s0=" + getS0() +
                '}';
    }
}
