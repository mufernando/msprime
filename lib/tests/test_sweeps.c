/*
** Copyright (C) 2016-2020 University of Oxford
**
** This file is part of msprime.
**
** msprime is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** msprime is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with msprime.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testlib.h"

static void
verify_simple_genic_selection_trajectory(
    double start_frequency, double end_frequency, double alpha, double dt)
{
    int ret;
    msp_t msp;
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_default);
    sample_t samples[] = { { 0, 0.0 }, { 0, 0.0 } };
    tsk_table_collection_t tables;
    size_t j, num_steps;
    double *allele_frequency, *time;

    CU_ASSERT_FATAL(rng != NULL);
    ret = tsk_table_collection_init(&tables, 0);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    tables.sequence_length = 1.0;

    ret = msp_alloc(&msp, 2, samples, &tables, rng);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    ret = msp_set_simulation_model_sweep_genic_selection(
        &msp, 0.5, start_frequency, end_frequency, alpha, dt);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    ret = msp_initialise(&msp);
    CU_ASSERT_EQUAL_FATAL(ret, 0);

    /* compute the trajectory */
    ret = msp.model.params.sweep.generate_trajectory(
        &msp.model.params.sweep, &msp, &num_steps, &time, &allele_frequency);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    CU_ASSERT_FATAL(num_steps > 1);
    CU_ASSERT_EQUAL(time[0], 0);
    CU_ASSERT_EQUAL(allele_frequency[0], end_frequency);
    CU_ASSERT_TRUE(allele_frequency[num_steps - 1] == start_frequency);

    for (j = 0; j < num_steps; j++) {
        CU_ASSERT_TRUE(allele_frequency[j] >= 0);
        CU_ASSERT_TRUE(allele_frequency[j] <= 1);
        if (j > 0) {
            CU_ASSERT_DOUBLE_EQUAL_FATAL(time[j], time[j - 1] + dt, 1e-9);
        }
    }

    free(time);
    free(allele_frequency);
    msp_free(&msp);
    tsk_table_collection_free(&tables);
    gsl_rng_free(rng);
}

static void
test_genic_selection_trajectory(void)
{
    verify_simple_genic_selection_trajectory(0.1, 0.9, 0.1, 0.0125);
    verify_simple_genic_selection_trajectory(0.1, 0.9, 0.01, 0.00125);
    verify_simple_genic_selection_trajectory(0.8999, 0.9, 0.1, 0.2);
    verify_simple_genic_selection_trajectory(0.1, 0.9, 100, 0.1);
    verify_simple_genic_selection_trajectory(0.1, 0.9, 1, 10);
}

static void
test_sweep_genic_selection_bad_parameters(void)
{
    int ret;
    msp_t msp;
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_default);
    sample_t samples[] = { { 0, 0.0 }, { 0, 0.0 } };
    tsk_table_collection_t tables;

    CU_ASSERT_FATAL(rng != NULL);
    ret = tsk_table_collection_init(&tables, 0);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    tables.sequence_length = 1.0;
    ret = msp_alloc(&msp, 2, samples, &tables, rng);
    CU_ASSERT_EQUAL_FATAL(ret, 0);

    ret = msp_set_simulation_model_sweep_genic_selection(
        &msp, 0.5, -0.01, 0.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_ALLELE_FREQUENCY);
    ret = msp_set_simulation_model_sweep_genic_selection(
        &msp, 0.5, 0.01, -0.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_ALLELE_FREQUENCY);
    ret = msp_set_simulation_model_sweep_genic_selection(
        &msp, 0.5, 10.01, 0.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_ALLELE_FREQUENCY);
    ret = msp_set_simulation_model_sweep_genic_selection(
        &msp, 0.5, 0.01, 10.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_ALLELE_FREQUENCY);

    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 0.5, 0.1, 0.01, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_TRAJECTORY_START_END);
    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 0.5, 0.1, 0.1, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_TRAJECTORY_START_END);

    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 0.5, 0.1, 0.9, 0.1, 0.0);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_TIME_DELTA);
    ret = msp_set_simulation_model_sweep_genic_selection(
        &msp, 0.5, 0.1, 0.9, 0.1, -0.01);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_TIME_DELTA);

    ret = msp_set_simulation_model_sweep_genic_selection(&msp, -0.5, 0.1, 0.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_SWEEP_POSITION);
    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 5.0, 0.1, 0.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_SWEEP_POSITION);

    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 0.5, 0.1, 0.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, 0);

    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 0.5, 0.1, 0.9, -666, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, MSP_ERR_BAD_SWEEP_GENIC_SELECTION_ALPHA);
    /* The incorrect number of populations was specified */
    ret = msp_set_dimensions(&msp, 2, 2);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_initialise(&msp);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_run(&msp, DBL_MAX, UINT32_MAX);
    CU_ASSERT_EQUAL(ret, MSP_ERR_UNSUPPORTED_OPERATION);

    msp_free(&msp);
    tsk_table_collection_free(&tables);
    gsl_rng_free(rng);
}

static void
test_sweep_genic_selection_events(void)
{
    int ret;
    msp_t msp;
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_default);
    sample_t samples[] = { { 0, 0.0 }, { 0, 0.0 }, { 0, 0.0 } };
    tsk_table_collection_t tables;

    CU_ASSERT_FATAL(rng != NULL);
    ret = tsk_table_collection_init(&tables, 0);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    tables.sequence_length = 1.0;

    ret = msp_alloc(&msp, 2, samples, &tables, rng);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 0.5, 0.1, 0.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    ret = msp_set_dimensions(&msp, 1, 2);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    ret = msp_add_population_parameters_change(&msp, 0.1, 0, 1, 0);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_initialise(&msp);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_run(&msp, DBL_MAX, UINT32_MAX);
    CU_ASSERT_EQUAL(ret, MSP_ERR_EVENTS_DURING_SWEEP);
    msp_free(&msp);

    tsk_table_collection_clear(&tables);
    samples[1].time = 0.1;
    ret = msp_alloc(&msp, 3, samples, &tables, rng);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 0.5, 0.1, 0.9, 0.1, 0.1);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    ret = msp_set_dimensions(&msp, 1, 2);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_initialise(&msp);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_run(&msp, DBL_MAX, UINT32_MAX);
    CU_ASSERT_EQUAL(ret, MSP_ERR_EVENTS_DURING_SWEEP);
    msp_free(&msp);

    tsk_table_collection_free(&tables);
    gsl_rng_free(rng);
}

static void
verify_sweep_genic_selection(double sequence_length, double growth_rate)
{
    int j, ret;
    uint32_t n = 10;
    unsigned long seed = 133;
    msp_t msp;
    sample_t *samples = malloc(n * sizeof(sample_t));
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_default);
    tsk_table_collection_t tables[2];

    CU_ASSERT_FATAL(samples != NULL);
    CU_ASSERT_FATAL(rng != NULL);
    memset(samples, 0, n * sizeof(sample_t));
    for (j = 0; j < 2; j++) {
        ret = tsk_table_collection_init(&tables[j], 0);
        CU_ASSERT_EQUAL_FATAL(ret, 0);
        tables[j].sequence_length = sequence_length;
        gsl_rng_set(rng, seed);
        ret = msp_alloc(&msp, n, samples, &tables[j], rng);
        CU_ASSERT_EQUAL(ret, 0);
        CU_ASSERT_EQUAL_FATAL(msp_set_recombination_rate(&msp, 1), 0);
        ret = msp_set_dimensions(&msp, 1, 2);
        CU_ASSERT_EQUAL(ret, 0);
        ret = msp_set_simulation_model_sweep_genic_selection(
            &msp, sequence_length / 2, 0.1, 0.9, 0.1, 0.01);
        CU_ASSERT_EQUAL(ret, 0);
        ret = msp_set_population_configuration(&msp, 0, 1.0, growth_rate);
        CU_ASSERT_EQUAL(ret, 0);
        ret = msp_initialise(&msp);
        CU_ASSERT_EQUAL(ret, 0);
        ret = msp_run(&msp, DBL_MAX, UINT32_MAX);
        CU_ASSERT_TRUE(ret >= 0);
        msp_print_state(&msp, _devnull);
        ret = msp_finalise_tables(&msp);
        CU_ASSERT_EQUAL(ret, 0);
        msp_free(&msp);
        CU_ASSERT_EQUAL(tables[j].migrations.num_rows, 0);
        CU_ASSERT(tables[j].nodes.num_rows > 0);
        CU_ASSERT(tables[j].edges.num_rows > 0);
    }
    CU_ASSERT_TRUE(tsk_node_table_equals(&tables[0].nodes, &tables[1].nodes));
    CU_ASSERT_TRUE(tsk_edge_table_equals(&tables[0].edges, &tables[1].edges));
    CU_ASSERT_EQUAL(ret, 0);
    gsl_rng_free(rng);
    free(samples);
    for (j = 0; j < 2; j++) {
        tsk_table_collection_free(&tables[j]);
    }
}

static void
test_sweep_genic_selection_single_locus(void)
{
    verify_sweep_genic_selection(1, 0.0);
    verify_sweep_genic_selection(1, 1.0);
    verify_sweep_genic_selection(1, -1.0);
}

static void
test_sweep_genic_selection_recomb(void)
{
    verify_sweep_genic_selection(100, 0.0);
    verify_sweep_genic_selection(100, 1.0);
    verify_sweep_genic_selection(100, -1.0);
}

static void
test_sweep_genic_selection_gc(void)
{
    int ret;
    uint32_t n = 100;
    unsigned long seed = 133;
    msp_t msp;
    sample_t *samples = malloc(n * sizeof(sample_t));
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_default);
    tsk_table_collection_t tables;

    memset(samples, 0, n * sizeof(sample_t));

    ret = tsk_table_collection_init(&tables, 0);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    tables.sequence_length = 10;
    gsl_rng_set(rng, seed);
    ret = msp_alloc(&msp, n, samples, &tables, rng);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL_FATAL(msp_set_recombination_rate(&msp, 1), 0);
    CU_ASSERT_EQUAL_FATAL(msp_set_gene_conversion_rate(&msp, 1), 0);
    CU_ASSERT_EQUAL_FATAL(msp_set_gene_conversion_track_length(&msp, 1), 0);
    ret = msp_set_dimensions(&msp, 1, 2);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_initialise(&msp);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_run(&msp, DBL_MAX, 100);
    CU_ASSERT_TRUE(msp_get_num_gene_conversion_events(&msp) > 0);
    CU_ASSERT_TRUE(ret >= 0);
    ret = msp_set_simulation_model_sweep_genic_selection(&msp, 5, 0.1, 0.9, 0.1, 0.01);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_run(&msp, DBL_MAX, UINT32_MAX);
    /* GC rate in sweep model is not implemented */
    CU_ASSERT_TRUE(ret == MSP_ERR_SWEEPS_GC_NOT_SUPPORTED);
    msp_print_state(&msp, _devnull);
    ret = msp_finalise_tables(&msp);
    CU_ASSERT_EQUAL(ret, 0);
    msp_free(&msp);

    gsl_rng_free(rng);
    free(samples);
    tsk_table_collection_free(&tables);
}

static void
test_sweep_genic_selection_time_change(void)
{
    int j, ret;
    uint32_t n = 10;
    double num_loci = 10;
    unsigned long seed = 133234;
    double t;
    msp_t msp;
    sample_t *samples = malloc(n * sizeof(sample_t));
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_default);
    tsk_table_collection_t tables;

    CU_ASSERT_FATAL(samples != NULL);
    CU_ASSERT_FATAL(rng != NULL);
    memset(samples, 0, n * sizeof(sample_t));

    ret = tsk_table_collection_init(&tables, 0);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    tables.sequence_length = num_loci;
    gsl_rng_set(rng, seed);
    ret = msp_alloc(&msp, n, samples, &tables, rng);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL_FATAL(msp_set_recombination_rate(&msp, 1), 0);
    ret = msp_set_dimensions(&msp, 1, 2);
    CU_ASSERT_EQUAL(ret, 0);
    ret = msp_initialise(&msp);
    CU_ASSERT_EQUAL(ret, 0);
    /* Run some time and then change the model */
    ret = msp_run(&msp, 0.125, UINT32_MAX);
    CU_ASSERT_EQUAL(ret, MSP_EXIT_MAX_TIME);
    t = msp_get_time(&msp);

    for (j = 0; j < 10; j++) {

        ret = msp_set_simulation_model_sweep_genic_selection(
            &msp, num_loci / 2, 0.1, 0.9, 0.1, 0.01);
        CU_ASSERT_EQUAL(ret, 0);

        CU_ASSERT_EQUAL(ret, 0);
        CU_ASSERT_EQUAL(msp_get_time(&msp), t);

        ret = msp_set_simulation_model_hudson(&msp);
        CU_ASSERT_EQUAL(ret, 0);
        CU_ASSERT_EQUAL(msp_get_time(&msp), t);
    }
    msp_free(&msp);

    gsl_rng_free(rng);
    free(samples);
    tsk_table_collection_free(&tables);
}

static void
sweep_genic_selection_mimic_msms_single_run(unsigned long int seed)
{

    /* Try to mimic the msms parameters used in verification.py
           "100 300 -t 200 -r 200 500000"
           " -SF 0 0.9 -Sp 0.5 -SaA 5000 -SAA 10000 -N 10000"
     */
    int ret;
    uint32_t n = 100;
    double num_loci = 500001;
    double position = 0.5;
    double alpha = 10000;
    double recom_rate = 0.0004;
    double start_frequency = 0.5 / 10000;
    double end_frequency = 0.9;
    double dt = 1.0 / 400000;

    msp_t msp;
    sample_t *samples = malloc(n * sizeof(sample_t));
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_default);
    tsk_table_collection_t tables;

    // Test over differnt seeds
    gsl_rng_set(rng, seed);

    CU_ASSERT_FATAL(samples != NULL);
    CU_ASSERT_FATAL(rng != NULL);
    memset(samples, 0, n * sizeof(sample_t));

    ret = tsk_table_collection_init(&tables, 0);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    tables.sequence_length = num_loci;
    ret = msp_alloc(&msp, n, samples, &tables, rng);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL_FATAL(msp_set_recombination_rate(&msp, recom_rate), 0);
    ret = msp_set_dimensions(&msp, 1, 2);
    CU_ASSERT_EQUAL(ret, 0);

    // To mimic the verfication.py call
    msp_set_discrete_genome(&msp, 0);
    msp_set_gene_conversion_rate(&msp, 0);
    msp_set_gene_conversion_track_length(&msp, 1);
    msp_set_avl_node_block_size(&msp, 65536);
    msp_set_node_mapping_block_size(&msp, 65536);
    msp_set_segment_block_size(&msp, 65536);

    ret = msp_set_simulation_model_sweep_genic_selection(
        &msp, position, start_frequency, end_frequency, alpha, dt);
    CU_ASSERT_EQUAL(ret, 0);

    ret = msp_initialise(&msp);
    CU_ASSERT_EQUAL(ret, 0);

    ret = msp_run(&msp, DBL_MAX, UINT32_MAX);
    CU_ASSERT_EQUAL(ret, 0);

    msp_verify(&msp, 0);

    msp_free(&msp);
    gsl_rng_free(rng);
    free(samples);
    tsk_table_collection_free(&tables);
}

static void
test_sweep_genic_selection_mimic_msms(void)
{
    /* To mimic the nrepeats = 300  parameter in msms cmdline arguments*/
    for (int i = 0; i < 300; i++)
        sweep_genic_selection_mimic_msms_single_run(i + 1);
}

int
main(int argc, char **argv)
{
    CU_TestInfo tests[] = {
        { "test_genic_selection_trajectory", test_genic_selection_trajectory },
        { "test_sweep_genic_selection_bad_parameters",
            test_sweep_genic_selection_bad_parameters },
        { "test_sweep_genic_selection_events", test_sweep_genic_selection_events },
        { "test_sweep_genic_selection_single_locus",
            test_sweep_genic_selection_single_locus },
        { "test_sweep_genic_selection_recomb", test_sweep_genic_selection_recomb },
        { "test_sweep_genic_selection_gc", test_sweep_genic_selection_gc },
        { "test_sweep_genic_selection_time_change",
            test_sweep_genic_selection_time_change },
        { "test_sweep_genic_selection_mimic_msms",
            test_sweep_genic_selection_mimic_msms },
        CU_TEST_INFO_NULL,
    };

    return test_main(tests, argc, argv);
}
