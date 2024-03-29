"use client";

import { JoystickModel } from "~/components/joystick-model";
import { Canvas } from "~/components/canvas";
import React, { useEffect, useState } from "react";
import { invoke } from "@tauri-apps/api/tauri";
import { Button } from "~/components/ui/button";

export interface Keybinding {
    name: string;
    action: string;
}

export type Keymap = Keybinding[];

export default function Home() {
    const [keymap, setKeymap] = useState<Keymap>();

    useEffect(() => {
        async function getData() {
            const resp = await invoke("get_keymap");
            console.log(resp);
            setKeymap(resp as any);
        }

        getData();
    }, []);

    return (
        <div className="h-[95vh] flex flex-col justify-center items-center">
            <Canvas>
                {keymap && (
                    <JoystickModel
                        keymap={keymap}
                        setKeymap={setKeymap as React.Dispatch<React.SetStateAction<Keymap>>}
                    />
                )}
            </Canvas>

            <Button onClick={() => invoke("save_keymap", { keymap: JSON.stringify(keymap) })}>Nahrát Keymapu</Button>
        </div>
    );
}
