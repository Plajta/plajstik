"use client";

import React, { useMemo } from "react";
import { Card, CardContent } from "~/components/ui/card";
import { ScrollArea } from "~/components/ui/scroll-area";
import { Separator } from "~/components/ui/separator";
import { keybinds } from "~/keybinds";

import type { Object3D } from "three";
import type { LabelMenu } from "~/components/joystick-model";

interface ButtonLabelProps {
    object: Object3D;
    labelMenus: LabelMenu[];
    setLabelMenus: React.Dispatch<React.SetStateAction<LabelMenu[]>>;
}

export function ButtonLabel({ object, labelMenus, setLabelMenus }: ButtonLabelProps) {
    const menu = useMemo(() => labelMenus.find((item) => item.id === object.uuid), [labelMenus]);

    return (
        <div className="annotation before:bg-slate-950">
            <Card className="shadow-none">
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
                    {object.userData.prop}
                </CardContent>
            </Card>

            {menu && menu.opened && (
                <Card className="shadow-none z-40 absolute left-36 bottom-[-150px] annotation-menu before:bg-slate-950">
                    <CardContent className="select-none p-0">
                        <h4 className="text-sm font-medium leading-none p-2">Keybindy</h4>

                        <Separator />

                        <ScrollArea className="h-72 w-48 p-2">
                            <div>
                                {keybinds.map((tag) => (
                                    <>
                                        <div key={tag.name} className="text-sm">
                                            {tag.label}
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
