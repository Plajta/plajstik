// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::{collections::HashMap, time::Duration};

use serde;
use serialport;

const LOAD_COMMAND: &str = "LOAD\u{04}";

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            load_keymap,
            save_keymap,
            available_ports
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

#[derive(serde::Serialize, serde::Deserialize)]
struct Keymap {
    version: i8,
    deadzone: f32,
    axes: HashMap<String, i16>,
    buttons: HashMap<String, i16>,
}

#[tauri::command(rename_all = "snake_case")]
fn load_keymap(port: String) -> Result<Keymap, String> {
    let mut handle = match serialport::new(port, 9600)
        .timeout(Duration::from_millis(10))
        .open()
    {
        Ok(handle) => handle,
        Err(_) => return Err("Connection timed out!".into()),
    };

    handle.write(LOAD_COMMAND.as_bytes()).unwrap();

    let mut response = String::new();

    handle.read_to_string(&mut response);

    match response
        .split_whitespace()
        .into_iter()
        .find(|l| l.starts_with("{"))
    {
        Some(keymap) => return Ok(serde_json::from_str(&keymap.replace("\x04", "")).unwrap()),
        None => return Err("Unknown board!".into()),
    }
}

#[tauri::command(rename_all = "snake_case")]
fn save_keymap(port: String, keymap: String) -> Result<bool, String> {
    let mut handle = match serialport::new(port, 9600)
        .timeout(Duration::from_millis(10))
        .open()
    {
        Ok(handle) => handle,
        Err(_) => return Err("Connection timed out!".into()),
    };

    handle
        .write(format!("{}\u{04}", keymap).as_bytes())
        .expect("aa");

    Ok(true)
}

#[tauri::command(rename_all = "snake_case")]
fn available_ports() -> Vec<String> {
    let ports = serialport::available_ports().expect("No ports found!");

    ports.into_iter().map(|f| f.port_name).collect()
}
