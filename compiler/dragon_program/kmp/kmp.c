#include <stdio.h>
#include <string.h>
#define LEN 100

int *MakePattern(char *pattern)
{
    int length = strlen(pattern);
    int *table = (int *)malloc(sizeof(int) * length);

    int p_prefix = 0;
    int p_suffix = 1;

    table[0] = 0;

    while (p_suffix < length)
    {
        while (p_prefix > 0 && pattern[p_prefix] != pattern[p_suffix])
            p_prefix = table[p_prefix - 1]; // because p_prefix and p_suffix always point to character which is matching
        if (pattern[p_prefix] == pattern[p_suffix])
        {
            p_prefix += 1;
            table[p_suffix] = p_prefix;
        }
        else
        {
            table[p_suffix] = 0;
        }
        p_suffix += 1;
    }

    return table;
}

int March(char *s, char *pattern, int *table)
{
    int p_s = 0;
    int p_pattern = 0;
    int pattern_len = strlen(pattern);

    while (p_s < strlen(s))
    {
        while (p_pattern > 0 && pattern[p_pattern] != s[p_s])
            p_pattern = table[p_pattern - 1];
        if (pattern[p_pattern] == s[p_s])
        {
            p_pattern += 1;
        }
        if (p_pattern == pattern_len)
            break;
        p_s += 1;
    }
    if (p_pattern == pattern_len)
        return p_s - pattern_len;
    else
        return -1;
}

int main(void)
{
    int pos = -1;
    char s[LEN] = "ABACDABABABACDABAABACDABAB";
                // ABACDABAB ABACDABA ABACDABABC
    // ABACDABAB ABACDABA ABACDAB ABACDA ABACDABABC
    char pattern[LEN] = "ABACDABABC"; // [0 0 1 0 0 1 2 3 2 0]

    // char s[LEN];
    // char pattern[LEN];
    //scanf("%s", s);
    //scanf("%s", pattern);

    int *table = MakePattern(pattern);
    pos = March(s, pattern, table);

    if (~pos)
    {
        printf("pos: %d\n", pos);
    }
    else
    {
        printf("not marched\n");
    }

    return 0;
}