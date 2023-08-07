package net.karpelevitch.radio;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static java.lang.Integer.parseInt;

class Operator {

    public final long[] take;

    Operator(String operator) {
        List<String> res = new ArrayList<>();
        String[] split = operator.split(",");
        for (String s : split) {
            String[] split1 = s.split(":");
            for (int i = 0; i < split1.length && i < 2; i++) {
                String s1 = split1[i];
                res.add(s1);
            }
        }
        take = new long[res.size()];
        for (int i = 0; i < take.length; i++) {
            take[i] = parseInt(res.get(i));
        }
    }
    Operator(long... take){
        this.take = take;
    }

    @Override
    public String toString() {
        return "take" + Arrays.toString(take);
    }
}
