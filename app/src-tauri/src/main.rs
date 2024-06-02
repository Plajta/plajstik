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
    pin_number: u16,
    action: String,
}

#[tauri::command(rename_all = "snake_case")]
fn get_keymap() -> Vec<Keybinding> {
    // todo: actually implementovat berani ze seriove sbernice

    let mut keybindings = Vec::new();

    keybindings.push(Keybinding {
        pin_number: 1,
        action: "a".to_string()
    });

    keybindings.push(Keybinding {
        pin_number: 2,
        action: "x".to_string()
    });

    keybindings.push(Keybinding {
        pin_number: 3,
        action: "select".to_string()
    });

    keybindings.push(Keybinding {
        pin_number: 4,
        action: "b".to_string()
    });

    keybindings.push(Keybinding {
        pin_number: 5,
        action: "a".to_string()
    });

    keybindings.push(Keybinding {
        pin_number: 6,
        action: "x".to_string()
    });

    keybindings.push(Keybinding {
        pin_number: 7,
        action: "y".to_string()
    });

    keybindings.push(Keybinding {
        pin_number: 8,
        action: "start".to_string()
    });

    keybindings.push(Keybinding {
        pin_number: 9,
        action: "start".to_string()
    });

    keybindings
}

#[tauri::command(rename_all = "snake_case")]
fn save_keymap(keymap: String) -> bool {
    let keybindings: Vec<Keybinding> = serde_json::from_str(&keymap).unwrap();

    for keybinding in keybindings {
        println!("{}", keybinding.pin_number);
    }

    return true
}