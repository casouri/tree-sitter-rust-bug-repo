(with-temp-buffer
  (insert "fn foo() -> i32 {
    4
}

fn main() {
    println!(\"{:?}\", foo());
}
")
  (cl-assert (equal (buffer-substring 52 67)
                    "(\"{:?}\", foo())"))
  (let ((parser (treesit-parser-create 'rust)))
    (treesit-parser-set-included-ranges parser '((52 . 67)))
    (cl-assert (equal (treesit-parser-included-ranges parser)
                      '((52 . 67))))

    (treesit-query-capture
     parser
     '((call_expression
        function:
        (identifier) @font-lock-function-call-face)))

    (goto-char 68)
    (insert " ")
    (treesit-query-capture
     parser
     '((call_expression
        function:
        (identifier) @font-lock-function-call-face)))))
