"use client";

import React, { useMemo } from "react";
import { CheckIcon } from "lucide-react";
import { Card, CardContent } from "~/components/ui/card";
import { ScrollArea } from "~/components/ui/scroll-area";
import { Separator } from "~/components/ui/separator";
import { keybinds, buttons } from "~/constants";

import type { Object3D } from "three";
import type { LabelMenu } from "~/components/joystick-model";
import type { Keymap } from "~/app/page";

interface ButtonLabelProps {
    keymap: Keymap;
    setKeymap: React.Dispatch<React.SetStateAction<Keymap>>;
    object: Object3D;
    labelMenus: LabelMenu[];
    setLabelMenus: React.Dispatch<React.SetStateAction<LabelMenu[]>>;
}

export function ButtonLabel({ keymap, setKeymap, object, labelMenus, setLabelMenus }: ButtonLabelProps) {
    const menu = useMemo(() => labelMenus.find((item) => item.id === object.uuid), [labelMenus]);
    const button = useMemo(() => buttons.find((button) => button.name === object.userData.prop), [object]);
    const keybinding = useMemo(
        () =>
            keybinds.find(
                (bind) => bind.name === keymap.keybindings.find((item) => item.name === button?.name)?.action,
            ),
        [keymap],
    );

    return (
        <div className="annotation before:bg-slate-950">
            <Card className="shadow-none w-52 text-center">
                <CardContent
                    className="select-none p-2"
                    onClick={() =>
                        setLabelMenus((prev) =>
                            prev.map((item) => {
                                if (item.id === object.uuid) {
                                    item.opened = !item.opened;
                                } else {
                                    item.opened = false;
                                }

                                return item;
                            }),
                        )
                    }
                >
                    {button && button.label} - {keybinding?.label}
                </CardContent>
            </Card>

            {menu && button && keybinding && menu.opened && (
                <Card className="shadow-none z-40 absolute left-72 bottom-[-150px] annotation-menu before:bg-slate-950">
                    <CardContent className="select-none p-0">
                        <h4 className="text-sm font-medium leading-none p-2">Keybindy</h4>

                        <Separator />

                        <ScrollArea className="h-72 w-48 p-2">
                            <div>
                                {keybinds.map((tag) => (
                                    <>
                                        <div
                                            key={tag.name}
                                            className="text-sm flex gap-2 justify-between cursor-pointer"
                                            onClick={() =>
                                                setKeymap((prev) => ({
                                                    ...prev,
                                                    keybindings: prev.keybindings.map((bind) => {
                                                        if (bind.name === button.name) {
                                                            bind.action = tag.name;
                                                        }

                                                        return bind;
                                                    }),
                                                }))
                                            }
                                        >
                                            {tag.label}

                                            {tag.name === keybinding.name && <CheckIcon className="h-5 w-5" />}
                                        </div>

                                        <Separator className="my-2" />
                                    </>
                                ))}
                            </div>
                        </ScrollArea>
                    </CardContent>
                </Card>
            )}
        </div>
    );
}
