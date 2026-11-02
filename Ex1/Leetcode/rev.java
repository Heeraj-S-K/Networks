//String Processing - Reverse Words in a String

public static String reverseWords(String s) {
    String[] a = s.trim().split("\\s+");
    String r = "";
    for (int i = a.length - 1; i >= 0; i--) {
        r += a[i];
        if (i > 0) r += " ";
    }
    return r;
}

Sample 1:

Input

OpenAl creates amazing Al models

Output

models Al amazing creates OpenAl

Sample 2:

Input

Data Science is fun

Output
fun is Science Data
