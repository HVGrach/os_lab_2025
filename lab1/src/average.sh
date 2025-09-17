if [ $# -eq 0 ]; then
echo "Usage: $0 num1 num2 ..." >&2
exit 1
fi
awk -v argc="$#" 'BEGIN{
  sum=0
  for (i=1;i<=argc;i++){
    x=ARGV[i]
    if (x !~ /^-?[0-9]+([.][0-9]+)?$/){
      printf("Non-numeric arg: %s\n", x) > "/dev/stderr"; exit 2
    }
    sum += x+0; ARGV[i]=""
  }
  printf("count=%d\naverage=%.6f\n", argc, sum/argc)
  exit
}' "$@"
