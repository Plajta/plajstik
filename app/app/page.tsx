"use client";

import { JoystickModel } from "~/components/joystick-model";
import { Canvas } from "~/components/canvas";
import { useEffect, useState } from "react";
import { invoke } from "@tauri-apps/api/tauri";

export interface Keymap {
    label: string;
    keybindings: Record<string, string>;
}

export default function Home() {
    const [keymap, setKeymap] = useState<Keymap>();

    useEffect(() => {
        async function getData() {
            const resp = await invoke("get_keymap");

            setKeymap(resp as any);
        }

        getData();
    }, []);

    return (
        <div className="h-[90vh] flex justify-center items-center">
            <Canvas>{keymap && <JoystickModel keymap={keymap} />}</Canvas>
        </div>
    );
}
