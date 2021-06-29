#!/bin/sh

test_description='basic tests for the oidtree implementation'
. ./test-lib.sh

echoid () {
	prefix="${1:+$1 }"
	shift
	while test $# -gt 0
	do
		echo "$1"
		shift
	done | awk -v prefix="$prefix" -v ZERO_OID=$ZERO_OID '{
		printf("%s%s", prefix, $0);
		need = length(ZERO_OID) - length($0);
		for (i = 0; i < need; i++)
			printf("0");
		printf "\n";
	}'
}

test_expect_success 'oidtree insert and contains' '
	cat >expect <<EOF &&
0
0
0
1
1
0
EOF
	{
		echoid insert 444 1 2 3 4 5 a b c d e &&
		echoid contains 44 441 440 444 4440 4444
		echo destroy
	} | test-tool oidtree >actual &&
	test_cmp expect actual
'

test_expect_success 'oidtree each' '
	echoid "" 123 321 321 >expect &&
	{
		echoid insert f 9 8 123 321 a b c d e
		echo each 12300
		echo each 3211
		echo each 3210
		echo each 32100
		echo destroy
	} | test-tool oidtree >actual &&
	test_cmp expect actual
'

test_done
