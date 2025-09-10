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

cat $TAB_C | tr -d '\n' | sed "s/.*yypact\[\] ={\([^}]*\)};.*/\t\"yypact\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yytable\[\] ={\([^}]*\)};.*/\t\"yytable\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*check\[\] ={\([^}]*\)};.*/\t\"yycheck\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yyr1\[\] ={\([^}]*\)};.*/\t\"yyr1\": [\1],/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json
cat $TAB_C | tr -d '\n' | sed "s/.*yyr2\[\] ={\([^}]*\)};.*/\t\"yyr2\": [\1]/" | sed "s/  */ /g" >> hfml.json
echo >> hfml.json


echo "}" >> hfml.json