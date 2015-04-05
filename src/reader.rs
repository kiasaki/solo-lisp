use std::borrow::ToOwned;
use types::SoloError::{ErrString, ErrSoloVal};
use types::{SoloVal, SoloRet,
            _nil, _true, _false, _int, symbol, string, list, vector, hash_mapv,
            err_str, err_string, err_val};
use super::printer::unescape_str;

#[derive(Debug, Clone)]
struct Reader {
    tokens: Vec<String>,
    position: usize,
}

impl Reader {
    fn next(&mut self) -> Option<String> {
        if self.position < self.tokens.len() {
            self.position += 1;
            Some(self.tokens[self.position-1].to_string())
        } else {
            None
        }
    }
    fn peek(&self) -> Option<String> {
        if self.position < self.tokens.len() {
            Some(self.tokens[self.position].to_string())
        } else {
            None
        }
    }
}

fn tokenize(str: String) -> Vec<String> {
    let mut results = vec![];
    let re = regex!(r###"[\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"|;.*|[^\s\[\]{}('"`,;)]*)"###);
    for cap in re.captures_iter(&str) {
        let group = cap.at(1).unwrap_or("");
        if group == "" { break; }
        if group.starts_with(";") { continue; }
        results.push(group.to_owned());
    }
    results
}

fn read_atom(rdr : &mut Reader) -> SoloRet {
    let otoken = rdr.next();
    if otoken.is_none() { return err_str("read_atom underflow"); }
    let stoken = otoken.unwrap();
    let token = &stoken[..];
    if regex!(r"^-?[0-9]+$").is_match(token) {
        let num : Option<isize> = token.parse().ok();
        Ok(_int(num.unwrap()))
    } else if regex!(r#"^".*"$"#).is_match(token) {
        let new_str = &token[1..token.len()-1];
        Ok(string(unescape_str(new_str)))
    } else if regex!(r#"^:"#).is_match(token) {
        Ok(string(format!("\u{29e}{}", &token[1..])))
    } else if token == "nil" {
        Ok(_nil())
    } else if token == "true" {
        Ok(_true())
    } else if token == "false" {
        Ok(_false())
    } else {
        Ok(symbol(token))
    }
}

fn read_seq(rdr : &mut Reader, start: &str, end: &str) -> Result<Vec<SoloVal>,String> {
    let otoken = rdr.next();
    if otoken.is_none() {
        return Err("read_atom underflow".to_string());
    }
    let stoken = otoken.unwrap();
    let token = &stoken[..];
    if token != start {
        return Err(format!("expected '{}'", start))
    }

    let mut ast_vec : Vec<SoloVal> = vec![];
    loop {
        let otoken = rdr.peek();
        if otoken.is_none() {
            return Err(format!("expected '{}', got EOF", end));
        }
        let stoken = otoken.unwrap();
        let token = &stoken[..];
        if token == end { break; }

        match read_form(rdr) {
            Ok(mv) => ast_vec.push(mv),
            Err(ErrString(es)) => return Err(es),
            Err(ErrSoloVal(_)) => return Err("read_seq exception".to_string()),
        }
    }
    rdr.next();

    Ok(ast_vec)
}

fn read_list(rdr : &mut Reader) -> SoloRet {
    match read_seq(rdr, "(", ")") {
        Ok(seq) => Ok(list(seq)),
        Err(es) => err_string(es),
    }
}

fn read_vector(rdr : &mut Reader) -> SoloRet {
    match read_seq(rdr, "[", "]") {
        Ok(seq) => Ok(vector(seq)),
        Err(es) => err_string(es),
    }
}

fn read_hash_map(rdr : &mut Reader) -> SoloRet {
    match read_seq(rdr, "{", "}") {
        Ok(seq) => hash_mapv(seq),
        Err(es) => err_string(es),
    }
}

fn read_form(rdr : &mut Reader) -> SoloRet {
    let otoken = rdr.peek();
    let stoken = otoken.unwrap();
    let token = &stoken[..];
    match token {
        "'" => {
            let _ = rdr.next();
            match read_form(rdr) {
                Ok(f) => Ok(list(vec![symbol("quote"), f])),
                Err(e) => Err(e),
            }
        },
        "`" => {
            let _ = rdr.next();
            match read_form(rdr) {
                Ok(f) => Ok(list(vec![symbol("quasiquote"), f])),
                Err(e) => Err(e),
            }
        },
        "~" => {
            let _ = rdr.next();
            match read_form(rdr) {
                Ok(f) => Ok(list(vec![symbol("unquote"), f])),
                Err(e) => Err(e),
            }
        },
        "~@" => {
            let _ = rdr.next();
            match read_form(rdr) {
                Ok(f) => Ok(list(vec![symbol("splice-unquote"), f])),
                Err(e) => Err(e),
            }
        },
        "^" => {
            let _ = rdr.next();
            match read_form(rdr) {
                Ok(meta) => {
                    match read_form(rdr) {
                        Ok(f) => Ok(list(vec![symbol("with-meta"), f, meta])),
                        Err(e) => Err(e),
                    }
                },
                Err(e) => Err(e),
            }
        },
        "@" => {
            let _ = rdr.next();
            match read_form(rdr) {
                Ok(f) => Ok(list(vec![symbol("deref"), f])),
                Err(e) => Err(e),
            }
        },

        ")" => err_str("unexected ')'"),
        "(" => read_list(rdr),

        "]" => err_str("unexected ']'"),
        "[" => read_vector(rdr),

        "}" => err_str("unexected '}'"),
        "{" => read_hash_map(rdr),

        _   => read_atom(rdr)
    }
}

pub fn read_str(str :String) -> SoloRet {
    let tokens = tokenize(str);
    if tokens.len() == 0 {
        // any soloval as the error slot means empty line
        return err_val(_nil())
    }
    //println!("tokens: {}", tokens);
    let rdr = &mut Reader{tokens: tokens, position: 0};
    read_form(rdr)
}
