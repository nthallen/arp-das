/'.'/ {
  for (i=1; i <= NF; i++) {
    if (match($i, "^'.'$")) cc[$i] = 1
  }
}
END {
  for (i in cc) print i
}
