// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::collections::HashMap;
use serde;

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![get_keymap])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

#[derive(serde::Serialize)]
struct Keymap {
    label: String,
    keybindings: HashMap<String, String>,
}

#[tauri::command(rename_all = "snake_case")]
fn get_keymap() -> Keymap {
    // todo: actually implementovat berani ze seriove sbernice

    let mut keybindings = HashMap::new();

    keybindings.insert("btn1".to_string(), "x".to_string());
    keybindings.insert("btn2".to_string(), "y".to_string());
    keybindings.insert("btn3".to_string(), "select".to_string());

    Keymap {
        label: "Default".to_string(),
        keybindings,
    }
}