"use client";

import { Object3D, Object3DEventMap } from "three";
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from "~/components/ui/card";

export function ButtonLabel({ object }: { object: Object3D<Object3DEventMap> }) {
    return (
        <div className="annotation">
            <Card className="shadow-none">
                <CardContent className="select-none p-2">{object.userData.prop}</CardContent>
            </Card>

            <Card className="shadow-none relative left-32 bottom-32">
                <CardContent className="select-none p-2">aaaaa</CardContent>
            </Card>
        </div>
    );
}
