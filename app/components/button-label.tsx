"use client";

import React, { useMemo } from "react";
import { Card, CardContent } from "~/components/ui/card";
import { ScrollArea } from "~/components/ui/scroll-area";
import { Separator } from "~/components/ui/separator";
import type { Object3D } from "three";
import type { LabelMenu } from "~/components/joystick-model";

interface ButtonLabelProps {
    object: Object3D;
    labelMenus: LabelMenu[];
    setLabelMenus: React.Dispatch<React.SetStateAction<LabelMenu[]>>;
}

const tags = Array.from({ length: 50 }).map((_, i, a) => `v1.2.0-beta.${a.length - i}`);

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
                                {tags.map((tag) => (
                                    <>
                                        <div key={tag} className="text-sm">
                                            {tag}
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
