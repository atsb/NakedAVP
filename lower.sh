#/bin/bash
for i in *
do
        d=$(echo ${i} |tr [:upper:] [:lower:]);
        if [ ${d} != ${i} ]
        then
                echo "renaming:" ${i} ${d}
                rename ${i} ${d} ${i}
#               mv ${i} ${d}
        fi
done
