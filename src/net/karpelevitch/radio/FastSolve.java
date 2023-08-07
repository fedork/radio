package net.karpelevitch.radio;

import java.util.ArrayList;
import java.util.List;

public class FastSolve {

    public static class Layer {

        private final int k;
        private Solution sa;

        public Layer(int k) {

            this.k = k;
        }

        public void setSa(Solution sa) {
            this.sa = sa;
        }

        public Solution getSa() {
            return sa;
        }
    }

    private static final List<Layer> layers = new ArrayList<>();

    public static final double RATIO_1 = 1.0d / Math.sqrt(3.0d);
    public static final double RATIO_2 = 1.0d / Math.sqrt(3.0d * (Math.sqrt(3.0d) - 1));

    public static void main(String[] args) {
        System.out.println("RATIO_1 = " + RATIO_1);
        System.out.println("RATIO_2 = " + RATIO_2);
        Layer l0 = new Layer(0);
        layers.add(l0); // zero
        Layer l1 = new Layer(1);
        layers.add(l1); // zero
        Solution t = new Solution(new Sa(2), 1, null, null, null, null);
        l0.setSa(t);
        l1.setSa(t);
        int Na = 3;
        while (Na<100000L) {
            int k = layers.size();
            Layer l = new Layer(k);
            layers.add(l);
            while (true) {
                Solution sol = solveSa(Na, k);
                if (sol == null) {
                    break;
                }
                l.setSa(sol);
                Na++;
            }
            System.out.println("Best solution for " + k + ": " + l.getSa().init);
        }
    }

    private static List<List<Long>> pareto = new ArrayList<>();
    private static void computePareto() {
        do {
            int k = pareto.size();

            List<Long> prev = k>0?pareto.get(k-1):null;
            List<Long> p = new ArrayList<>();
            int n2 = 0;
            int n1 = 0;
            do {
                n2++;
                switch (n2) {
                    case 1: n1 = 1<<k; break;
                    case 2: n1 = (1<<k) - 1; break;
                    case 3: n1 = (1<<k) - k; break;
                    case 4: n1 = (1<<k) - 2*k + 2; break; // also n1 = prev.get(3) * 2
//                    case 5:
                    default:
                 }

                p.add(n2, n1);
            } while (n1-n2>1);
            pareto.add(p);
        } while (true);
    }

    private static Solution solveSa(int n, int k) {
        if (n <= 2) return Solution.TRIVIAL;
        Sa prevBest = (Sa) layers.get(k - 1).getSa().init;
        int rem = n - prevBest.count;
        Sb sb = new Sb(prevBest.count, rem);
        Sa init = new Sa(n);
        if (solveSb(sb, k - 1) == null) return null;
//        System.out.println("can solve " + init + " in " + k);
        return new Solution(init, k, new Operator(prevBest.count), prevBest, sb, new Sa(rem));
    }

    private static Solution solveSb(Sb init, int k) {
        if (init.pairs > getInfoLimit(k)) {
            System.out.println("can't solve " + init + " in " + k + " (info limit)");
            return null;
        }
        Sb normalized = init.normalize();
        if (normalized.isTrivial()) return Solution.TRIVIAL;
//        System.out.println("solving " + normalized + " in " + k);
        Sb s2 = new Sb();
        Sb s0 = new Sb();
        Sb s1 = new Sb();
        Operator op = new Operator(new long[normalized.groups.size()*2]);
        for (int i = 0; i < normalized.groups.size(); i++) {
            Sb.Group group = normalized.groups.get(i);
            long fr = group.pairs * 1000L / getInfoLimit(k);
            long m1;
            long m2;
            long n1 = group.a;
            long n2 = group.b;
            if (normalized.groups.size() == 1 || fr > 400L) {
                if (n2>4 && n1<2.5d*n2) {
                    long prevBest = ((Sa) layers.get(k - 1).getSa().init).count;
                    long thisBest = ((Sa) layers.get(k).getSa().init).count;
                    double prod = (thisBest - prevBest) * prevBest;
                    m1 = Math.min(n1, Math.round(Math.sqrt(prod * n1 / n2)));
                    m2 = Math.min(n2, Math.round(prod / m1));

//                    long m = Math.round(RATIO_1 * (n1+n2));
//                    m1 = Math.round((double) n1 * m/(n1+n2));
//                    m2 = m-m1;
                } else {
                    m1 = (n1+1)/2;
                    m2 = Math.round(2.0d*n2/3.0d);
                }

            } else if (/*normalized.groups.size() == 2 || */fr > 250L) { // more than a quarter
                if (n2>3) {
                    long prevBest = ((Sa) layers.get(k - 1).getSa().init).count;
                    long thisBest = ((Sa) layers.get(k).getSa().init).count;
                    double prod = (thisBest - prevBest) * prevBest;
                    m1 = Math.min(n1, Math.round(Math.sqrt(prod * n1 / n2)));
                    m2 = Math.min(n2, Math.round(prod / m1));
                } else {
                    long m = ((2L<<k)/4L) + 1L;
                    m2 = Math.min(2L, n2);
                    m1 = Math.min(m - m2, n1);
                }

            } else {
                long pairsLeft1 = getInfoLimit(k - 1) - s1.pairs;
                if (group.pairs * 1000L < 900L * pairsLeft1) {
                    // just starting
                    if (n2>2) {
                        m1 = (n1 + 1L) / 2;
                        m2 = Math.round((double) m1 * n2 / n1);
                    } else {
                        m1 = (n1+1)/2;
                        m2 = 1;
                    }
                } else {
                    long targ1;
                    if (i<normalized.groups.size()-1 && normalized.groups.get(i+1).pairs*2L > group.pairs) {
                        // next is comparable
                        targ1 = pairsLeft1/2;
                    } else {
                        targ1 = pairsLeft1;
                    }
                    if (targ1<n1) {
                        m2=n2;
                        m1=n1-targ1/m2;
                    } else {
                        double r = 0.5d + 0.5d * Math.sqrt(1.0d - Math.pow(targ1, 2) / (n1 * n1 * n2 * n2));
                        m1 = Math.round(n1 * r);
                        m2 = Math.round(r * (n1 + n2)) - m1;
                    }
                }
            }
            if (s2.pairs > s0.pairs) {
                m1 = n1 - m1;
                m2 = n2 - m2;
            }
//            System.out.println("group = " + group);
//            System.out.println("m1 = " + m1);
//            System.out.println("m2 = " + m2);
            s2.add(m1,m2);
            s0.add(n1-m1, n2-m2);
            s1.add(n1-m1, m2);
            s1.add(m1, n2-m2);
            op.take[i*2] = m1;
            op.take[i*2+1] = m2;
        }
        if (solveSb(s2, k - 1) != null
                && solveSb(s0, k - 1) != null
                && solveSb(s1, k - 1) != null) {
            return new Solution(normalized, k, op, s2, s1, s0);
        }
        System.out.println("can't solve " + normalized + " in " + k + " with " + op);
        return null;
    }

    private static List<Long> infoLimit = new ArrayList<Long>() {{
        add(1L);
    }};

    private static long getInfoLimit(int k) {
        if (infoLimit.size() > k) return infoLimit.get(k);
        long limit = 3L * getInfoLimit(k - 1);
        infoLimit.add(k, limit);
        return limit;
    }

}
