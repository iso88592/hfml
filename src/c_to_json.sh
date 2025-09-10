#!/bin/bash

TAB_C=$(find ../ -name "hfml.tab.c" | tail -n 1)

echo "{" > hfml.json

enums=$(cat $TAB_C | tr -d '\n' | sed "s/.*yysymbol_kind_t{\([^}]*\)};.*/\1,/" | sed "s%/\*[^*]*\*/%%g" | sed "s/ //g")
#echo "$enums";

echo -e "\t\"tokens\":[" >>hfml.json
IFS=',' 
count=0
for s in $enums; do
    ((count++))
    if [[ $count -gt 1 ]]; then
        echo "," >>hfml.json
    fi
    echo -e "\t\t{" >>hfml.json
    echo -e $s | sed "s/YYSYMBOL_\([^=]*\).*/\t\t\t\"name\": \"\1\",/" >>hfml.json
    echo -e $s | sed "s/.*=\(.*\)/\t\t\t\"id\": \"\1\"/" >>hfml.json
    echo -en "\t\t}" >>hfml.json
done

echo -e "\n\t],">>hfml.json

cat $TAB_C | tr -d '\n' | sed "s/.*YYPACT_NINF (\([^)]*\)).*/\t\"yyninf\": \1,/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | grep "define YYFINAL" | sed "s/.*#define YYFINAL \(.*\)/\t\"yyfinal\": \1,/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | grep "define YYLAST" | sed "s/.*#define YYLAST \(.*\)/\t\"yylast\": \1,/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | grep "define YYNTOKENS" | sed "s/.*#define YYNTOKENS \(.*\)/\t\"yyntokens\": \1,/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | grep "define YYNNTS" | sed "s/.*#define YYNNTS \(.*\)/\t\"yynnts\": \1,/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | grep "define YYNRULES" | sed "s/.*#define YYNRULES \(.*\)/\t\"yynrules\": \1,/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | grep "define YYNSTATES" | sed "s/.*#define YYNSTATES \(.*\)/\t\"yynstates\": \1,/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yydefact\[\] ={\([^}]*\)};.*/\t\"yydefact\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yypact\[\] ={\([^}]*\)};.*/\t\"yypact\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yypgoto\[\] ={\([^}]*\)};.*/\t\"yypgoto\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yydefgoto\[\] ={\([^}]*\)};.*/\t\"yydefgoto\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yytable\[\] ={\([^}]*\)};.*/\t\"yytable\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*check\[\] ={\([^}]*\)};.*/\t\"yycheck\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yystos\[\] ={\([^}]*\)};.*/\t\"yystos\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json

cat $TAB_C | tr -d '\n' | sed "s/.*yyr1\[\] ={\([^}]*\)};.*/\t\"yyr1\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json


cat $TAB_C | tr -d '\n' | sed "s/.*yyr2\[\] ={\([^}]*\)};.*/\t\"yyr2\": [\1]/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json

echo "}" >> hfml.json