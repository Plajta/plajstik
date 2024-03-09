// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use serde;

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![get_keymap])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

#[derive(serde::Serialize)]
struct Keybinding {
    name: String,
    action: String,
}

#[derive(serde::Serialize)]
struct Keymap {
    label: String,
    keybindings: Vec<Keybinding>,
}

#[tauri::command(rename_all = "snake_case")]
fn get_keymap() -> Keymap {
    // todo: actually implementovat berani ze seriove sbernice

    let mut keybindings = Vec::new();

    keybindings.push(Keybinding {
        name: "btn1".to_string(),
        action: "a".to_string()
    });

    keybindings.push(Keybinding {
        name: "btn2".to_string(),
        action: "x".to_string()
    });

    keybindings.push(Keybinding {
        name: "btn3".to_string(),
        action: "select".to_string()
    });

    Keymap {
        label: "Default".to_string(),
        keybindings,
    }
}