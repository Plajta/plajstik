"use client";

import { useGLTF, Html } from "@react-three/drei";
import { ButtonLabel } from "~/components/button-label";
import React, { useEffect, useState } from "react";
import { Object3D } from "three";
import { Keymap } from "~/app/page";

export interface LabelMenu {
    id: string;
    opened: boolean;
}

interface JoystickModelProps {
    keymap: Keymap;
    setKeymap: React.Dispatch<React.SetStateAction<Keymap>>;
}

export function JoystickModel({ keymap, setKeymap }: JoystickModelProps) {
    const { scene } = useGLTF("/models/joystick.glb");

    const [labelMenus, setLabelMenus] = useState<LabelMenu[]>([]);
    const [objects, setObjects] = useState<Object3D[]>([]);

    useEffect(
        () =>
            scene.traverse((o) => {
                if (o.userData.prop) {
                    setLabelMenus((prev) => [...prev, { id: o.uuid, opened: false }]);

                    setObjects((prev) => [...prev, o]);
                }
            }),
        [],
    );

    return (
        <primitive object={scene}>
            {objects.map((object) => (
                <Html
                    key={object.uuid}
                    position={[object.position.x, object.position.y, object.position.z]}
                    distanceFactor={0.25}
                >
                    <ButtonLabel
                        keymap={keymap}
                        setKeymap={setKeymap}
                        object={object}
                        labelMenus={labelMenus}
                        setLabelMenus={setLabelMenus}
                    />
                </Html>
            ))}
        </primitive>
    );
}
