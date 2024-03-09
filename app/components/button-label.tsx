"use client";

import { useState } from "react";
import { Object3D, Object3DEventMap } from "three";
import { Card, CardContent } from "~/components/ui/card";

export function ButtonLabel({ object }: { object: Object3D<Object3DEventMap> }) {
    const [bindingsOpen, setBindingsOpen] = useState<boolean>(false);

    return (
        <div className="annotation before:bg-slate-950">
            <Card className="shadow-none">
                <CardContent className="select-none p-2" onClick={() => setBindingsOpen((prev) => !prev)}>
                    {object.userData.prop}
                </CardContent>
            </Card>

            {bindingsOpen && (
                <Card className="shadow-none z-40 absolute left-36 bottom-28 annotation-menu before:bg-slate-950">
                    <CardContent className="select-none p-2">aaaaa</CardContent>
                </Card>
            )}
        </div>
    );
}
