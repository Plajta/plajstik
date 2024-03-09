"use client";

import { useGLTF, Html } from "@react-three/drei";
import { ButtonLabel } from "~/components/button-label";

export function JoystickModel() {
    const { scene } = useGLTF("/models/joystick.glb");

    const annotations: React.ReactNode[] = [];

    scene.traverse((o) => {
        if (o.userData.prop) {
            annotations.push(
                <Html key={o.uuid} position={[o.position.x, o.position.y, o.position.z]} distanceFactor={0.25}>
                    <ButtonLabel object={o} />
                </Html>,
            );
        }
    });

    return <primitive object={scene}>{annotations}</primitive>;
}
