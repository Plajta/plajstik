"use client";

import { JoystickModel } from "~/components/joystick-model";
import { Canvas } from "~/components/canvas";
import React, { useEffect, useRef, useState } from "react";
import { invoke } from "@tauri-apps/api/tauri";
import { Button } from "~/components/ui/button";
import { View, ViewSwitcher } from "~/components/view-switcher";
import { ThemeSwitcher } from "~/components/theme-switcher";
import { Editor } from "@monaco-editor/react";
import { useTheme } from "next-themes";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "~/components/ui/select";
import { toast } from "sonner";

export interface Keymap {
    version: number;
    deadzone: number;
    axes: Record<string, number>;
    buttons: Record<string, number>;
}

export default function Home() {
    const [keymap, setKeymap] = useState<Keymap>();
    const [ports, setPorts] = useState<string[]>();
    const [view, setView] = useState<View>("editor");
    const [selectedPort, setSelectedPort] = useState<string | null>(null);
    // bad practice ale I cannot be bothered anymore
    const [aa, setAa] = useState("");

    const { theme } = useTheme();

    const editorRef = useRef(null);

    function handleEditorDidMount(editor: any) {
        editorRef.current = editor;
    }

    useEffect(() => {
        async function getData() {
            const portsResponse = await invoke("available_ports");

            setPorts(portsResponse as any);
        }

        getData();
    }, []);

    return (
        <div className="h-[95vh] flex mt-2 flex-col gap-2">
            <div className="flex justify-between">
                <ViewSwitcher view={view} setView={setView} />

                <ThemeSwitcher />
            </div>

            {view === "model" ? (
                <Canvas>
                    {keymap && (
                        <JoystickModel
                            keymap={keymap}
                            setKeymap={setKeymap as React.Dispatch<React.SetStateAction<Keymap>>}
                        />
                    )}
                </Canvas>
            ) : (
                <Editor
                    height="80%"
                    language="json"
                    theme={theme === "light" ? "light" : "vs-dark"}
                    value={JSON.stringify(keymap, null, "\t")}
                    onChange={(v) => v && setAa(JSON.parse(v))}
                    onMount={handleEditorDidMount}
                    options={{
                        inlineSuggest: { enabled: true },
                        fontSize: 16,
                        formatOnType: true,
                        autoClosingBrackets: "always",
                        minimap: { enabled: false },
                    }}
                />
            )}

            <div className="flex justify-center flex-grow gap-4 items-center">
                {ports && (
                    <Select onValueChange={(v) => setSelectedPort(v as any)}>
                        <SelectTrigger className="w-[180px]">
                            <SelectValue placeholder="Select Port" />
                        </SelectTrigger>
                        <SelectContent>
                            {ports.map((port) => (
                                <SelectItem key={`port-${port}`} value={port}>
                                    {port}
                                </SelectItem>
                            ))}
                        </SelectContent>
                    </Select>
                )}

                {selectedPort && (
                    <>
                        <Button
                            onClick={async () => {
                                if (!selectedPort) {
                                    console.log("port not selected!");
                                }

                                try {
                                    const response = await invoke("load_keymap", {
                                        port: selectedPort,
                                    });

                                    setKeymap(response as any);
                                } catch (e) {
                                    toast.error(e as any);
                                }
                            }}
                        >
                            Load Keymap
                        </Button>

                        <Button
                            onClick={async () => {
                                if (!selectedPort) {
                                    console.log("port not selected!");
                                }

                                try {
                                    await invoke("save_keymap", {
                                        port: selectedPort,
                                        keymap: JSON.stringify(aa),
                                    });

                                    toast.success("Successfully uploaded keymap!");
                                } catch (e) {
                                    toast.error(e as any);
                                }
                            }}
                        >
                            Save Keymap
                        </Button>
                    </>
                )}
            </div>
        </div>
    );
}
