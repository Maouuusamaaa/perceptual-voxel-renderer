
#[test]
fn perceptual_streaming_controller_keeps_spatial_id_stable_when_candidate_order_changes() {
    let index_a = PvrStreamIndex::new(vec![
        PvrStreamIndexEntry {
            dimension: "overworld".to_string(),
            chunk_x: 2,
            chunk_z: 0,
            section_y: 0,
            block_count: 1,
            payload_offset: 0,
            payload_size: 1,
        },
        PvrStreamIndexEntry {
            dimension: "overworld".to_string(),
            chunk_x: 1,
            chunk_z: 0,
            section_y: 0,
            block_count: 1,
            payload_offset: 1,
            payload_size: 1,
        },
    ]);

    let index_b = PvrStreamIndex::new(vec![
        PvrStreamIndexEntry {
            dimension: "overworld".to_string(),
            chunk_x: 1,
            chunk_z: 0,
            section_y: 0,
            block_count: 1,
            payload_offset: 1,
            payload_size: 1,
        },
        PvrStreamIndexEntry {
            dimension: "overworld".to_string(),
            chunk_x: 2,
            chunk_z: 0,
            section_y: 0,
            block_count: 1,
            payload_offset: 0,
            payload_size: 1,
        },
    ]);

    let perceptual_a = PvrPerceptualQuery::new(index_a);

    let prediction_a = PvrPredictionQuery::new(
        PvrStreamIndex::new(vec![
            PvrStreamIndexEntry {
                dimension: "overworld".to_string(),
                chunk_x: 2,
                chunk_z: 0,
                section_y: 0,
                block_count: 1,
                payload_offset: 0,
                payload_size: 1,
            },
            PvrStreamIndexEntry {
                dimension: "overworld".to_string(),
                chunk_x: 1,
                chunk_z: 0,
                section_y: 0,
                block_count: 1,
                payload_offset: 1,
                payload_size: 1,
            },
        ]),
    );

    let perceptual_b = PvrPerceptualQuery::new(index_b);

    let prediction_b = PvrPredictionQuery::new(
        PvrStreamIndex::new(vec![
            PvrStreamIndexEntry {
                dimension: "overworld".to_string(),
                chunk_x: 1,
                chunk_z: 0,
                section_y: 0,
                block_count: 1,
                payload_offset: 1,
                payload_size: 1,
            },
            PvrStreamIndexEntry {
                dimension: "overworld".to_string(),
                chunk_x: 2,
                chunk_z: 0,
                section_y: 0,
                block_count: 1,
                payload_offset: 0,
                payload_size: 1,
            },
        ]),
    );

    let camera = CameraState::new(
        0.0,
        0.0,
        0.0,
        1.0,
        0.0,
    );

    let controller = PerceptualStreamingController::new();

    let budgets = ResourceBudgets::new(vec![
        ResourceBudget::new(CacheResourceKind::Ram, 10.0),
        ResourceBudget::new(CacheResourceKind::Vram, 10.0),
    ]);

    let decisions_a = controller.build_camera_frame_decisions(
        &camera,
        &perceptual_a,
        &prediction_a,
        "overworld",
        PredictionCone::new(90.0),
        2,
        &budgets,
    );

    let decisions_b = controller.build_camera_frame_decisions(
        &camera,
        &perceptual_b,
        &prediction_b,
        "overworld",
        PredictionCone::new(90.0),
        2,
        &budgets,
    );

    let ids_a: Vec<u32> = decisions_a
        .iter()
        .map(|decision| decision.id())
        .collect();

    let ids_b: Vec<u32> = decisions_b
        .iter()
        .map(|decision| decision.id())
        .collect();

    assert_eq!(
        ids_a,
        ids_b,
        "the same spatial sections must keep the same IDs even when candidate input order changes"
    );
}
