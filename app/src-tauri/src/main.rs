// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use serde;

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![get_keymap, save_keymap])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

#[derive(serde::Serialize, serde::Deserialize   )]
struct Keybinding {
    name: String,
    action: String,
}

#[tauri::command(rename_all = "snake_case")]
fn get_keymap() -> Vec<Keybinding> {
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

    keybindings.push(Keybinding {
        name: "btn4".to_string(),
        action: "b".to_string()
    });

    keybindings.push(Keybinding {
        name: "btn5".to_string(),
        action: "a".to_string()
    });

    keybindings.push(Keybinding {
        name: "btn6".to_string(),
        action: "x".to_string()
    });

    keybindings.push(Keybinding {
        name: "btn7".to_string(),
        action: "y".to_string()
    });

    keybindings.push(Keybinding {
        name: "btn8".to_string(),
        action: "start".to_string()
    });

    keybindings.push(Keybinding {
        name: "dpad".to_string(),
        action: "start".to_string()
    });

    keybindings
}

#[tauri::command(rename_all = "snake_case")]
fn save_keymap(keymap: String) -> bool {
    let keybindings: Vec<Keybinding> = serde_json::from_str(&keymap).unwrap();

    for keybinding in keybindings {
        println!("{}", keybinding.name);
    }

    return true
}