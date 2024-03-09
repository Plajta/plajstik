"use client";

import React, { useMemo } from "react";
import { Object3D } from "three";
import { Card, CardContent } from "~/components/ui/card";
import { LabelMenu } from "~/components/joystick-model";

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
                <Card className="shadow-none z-40 absolute left-36 bottom-28 annotation-menu before:bg-slate-950">
                    <CardContent className="select-none p-2">aaaaa</CardContent>
                </Card>
            )}
        </div>
    );
}
