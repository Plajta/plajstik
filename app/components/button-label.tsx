"use client";

import { Object3D, Object3DEventMap } from "three";
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from "~/components/ui/card";

export function ButtonLabel({ object }: { object: Object3D<Object3DEventMap> }) {
    return (
        <div className="annotation before:bg-slate-950">
            <Card className="shadow-none">
                <CardContent className="select-none p-2">{object.userData.prop}</CardContent>
            </Card>

            <Card className="shadow-none z-40 relative left-32 bottom-32 annotation-menu before:bg-slate-950">
                <CardContent className="select-none p-2">aaaaa</CardContent>
            </Card>
        </div>
    );
}
